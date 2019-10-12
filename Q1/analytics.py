import subprocess
import random
import matplotlib.pyplot as plt
import re

def start(executable_file):
    return subprocess.Popen(
        executable_file,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

def read(process):
    return process.stdout.read().decode("utf-8").strip()

def write(process, message):
    process.stdin.write(f"{message.strip()}\n".encode("utf-8"))
    process.stdin.flush()

def terminate(process):
    process.stdin.close()
    process.terminate()
    process.wait(timeout=0.2)

subprocess.Popen("make")

strategy = '^Strategy: (.*)$'
time = 'Time taken: (.*)$'

n_inp = []
for i in range(7):
    n_inp.append(1 * (10 ** i))
    n_inp.append(5 * (10 ** i))

time_normal = []
time_proc = []
time_thread = []

for i in n_inp:
    inp = f"{i}\n"

    inp_list = list(range(1, i + 1))

    random.shuffle(inp_list)
    inp += ((' '.join([ str(x) for x in inp_list ])) + '\n')

    prog = start("./q1")

    write(prog, inp)

    out = read(prog).split("\n")

    norm = proc = thrd = -1

    for i in range(len(out)):
        x = re.match(strategy, out[i])
        if x:
            if x.groups()[0] == "Normal":
                norm = i
            elif x.groups()[0] == "Process":
                proc = i
            elif x.groups()[0] == "Thread":
                thrd = i

    if norm >= 0:
        n = re.match(time, out[norm + 1]).groups()[0]
        time_normal.append(float(n[:-1]))
    else:
        time_normal.append(-1)

    if proc >= 0:
        p = re.match(time, out[proc + 1]).groups()[0]
        time_proc.append(float(p[:-1]))

    if thrd >= 0:
        t = re.match(time, out[thrd + 1]).groups()[0]
        time_thread.append(float(t[:-1]))

    terminate(prog)

fig, ax = plt.subplots()
fig.canvas.draw()

ax.plot(time_normal, label="Normal")
ax.plot(time_proc, label="Process")
ax.plot(time_thread, label="Threaded")
plt.xticks([ i for i in range(len(n_inp)) ])
ax.set_xticklabels([ str(x) for x in n_inp ])
plt.xlabel("Input size")
plt.ylabel("Time taken(s)")
plt.title("Input size vs Time taken(s)")

ax.legend()
plt.show()
