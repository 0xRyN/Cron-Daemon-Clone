# Saturnd / Cassini - A FIFO Client / Server implementation

## Client :

The client `cassini` is just a bridge. It will parse user input, convert them according to `protocol.md` then send them to the `saturnd`'s request FIFO

It will then open the `saturnd`'s response pipe, and read the reponse sent by the server.

Each of the operations the client can send is in its standalone file.

We defined two structures, `command` and `cstring` (a string with it's length), for holding a command line.

## Server :

The server `saturnd` is way more complex.

Basically, it is a `poll` which checks the request fifo for requests (send by cassini).

It needs to fork() to run tasks. The Signal SIGCHLD will interrupt the poll, that's why we use the _self pipe trick_, so a fork() won't interrupt the poll.

It needs to create and store tasks in memory. We decided to store everything in
`/tmp/user/saturnd/tasks`

The `tasks` directory contains a subdirectory for each task id. Each contains :

1. Runs : Data about the past runs(errorcode and timing for -x)

2. Taskid : A file containing the taskid in big endian

3. Command : A file containing the command line to execute

4. Timing : A file containing the timing of the task (when to execute)

Each time we recieve a LIST_TASKS, we just iterate over the tasks dir with a `readdir()`.

Since the tasks need to be in order, and `readdir` doesn't guarantee an order, we need to :

1. Iterate over directory and put all id's in an int array (ex {2, 11, 5, 4})

2. Sort (quicksort) the array. This will give us valid task (ex {2, 4, 5, 11})

3. We iterate over the array. We know have all valid id's to list.

4. For each id, read and convert it's data. Send it over to cassini.

`saturnd` will poll over the request. If it has no request for 60 seconds, it will check all tasks :

1. Iterate over all tasks in the tasks dir. Read the timing of each task.

2. Send the timing to the `should_run_now` in the "timing-text-io" function. It will perform bitwise arithmetics (left shift) and boolean logic to determine if current time is equal to timing time.

3. If it should run, fork : the parent will wait "NOHANG" (continue poll), and the son will be forked again. The son of the son will fill the timing, dup2 into stdout and stderr file and run the task. The parent of the son will wait exitcode, and fill it.

Deleting a task will just remove the corresponding directory recursively.

The other commands are pretty trivial, and follow overall the same process.

Overall, the project does everything specified in the `enonce.md`.

Note : some memory leaks are present. Will need further testing to clear.
