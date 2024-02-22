import json

# parse bdc file

with open("tesla_radar.dbc") as f:
    lines = f.readlines()


pids = []
    
for line in lines:
    if line.startswith("BO_"):
        p = int(line.split()[1])
        
        pids.append(f"{p:0X}")
    

print(f"Total number of pids: {len(pids)}")

rates = [1, 10, 100]
chunks = len(rates)
chunk_len = len(pids) // chunks

cc = []

for i in range(chunks-1):
    cc.append(pids[i*chunk_len:(i+1)*chunk_len])
    
cc.append(pids[(chunks-1)*chunk_len:])

print(cc)

for c in cc:
    print(len(c))
    
commands = []

pid_timing = {}

for i, r in enumerate(rates):
    for c in cc[i]:
        pid_timing[int(c, base=16)] = r
        
json.dump(pid_timing, open("pid_timing.json", "w"), indent=4)

print(pid_timing)
           
