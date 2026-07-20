# Chord DHT

A small C++ implementation of **Chord**, the classic peer-to-peer algorithm for
building a Distributed Hash Table (DHT). A set of independent `node` processes
arrange themselves into a logical ring and cooperatively store key/value data,
so that any node can locate any key by forwarding a request a few hops around
the ring — no central server, no single point of failure.

A separate `client` program connects to any one node and can upload or search
for data on the ring.

## How it works

Every node and every piece of uploaded data is hashed with SHA-1 down to a
small integer ID (`gethash()` in `ChordNode/util.cpp`). This build uses a
4-bit ring, so IDs range **0–15** — small on purpose, so you can watch a whole
ring on screen during a local demo. (Production Chord typically uses 160-bit
IDs; the `m` parameter here is just hard-coded smaller.)

- **The ring.** Nodes are placed on a circle by ID. A key belongs to the first
  node whose ID is ≥ the key's hash, walking clockwise. Each node tracks a
  `successor` and `predecessor` (its neighbors) and a small `finger` table
  (routing shortcuts) in `ChordNode/nodeclass.h`.
- **Joining.** The first node runs `create_ring` and bootstraps a ring of one.
  Every other node runs `join <ip> <port>` against any existing member, which
  triggers a `findsuccessor` lookup to find where the new node belongs.
- **Staying consistent.** A background thread (`stable()` in
  `ChordNode/nodesync.cpp`) runs every 5 seconds on every node: it checks
  whether its successor's predecessor has changed (meaning someone joined in
  between), fixes its own pointers if so, and `notify`s its successor about
  itself. Whenever a node's predecessor changes, it also migrates any keys
  that now belong to that predecessor. This self-healing loop is what lets
  the ring converge correctly even as nodes join and leave at arbitrary
  times.
- **Upload / search.** The client hashes the string and sends `upload`/
  `search` to any node. That node checks "does this key fall in my range?" —
  if not, it forwards the request to its successor, and so on around the
  ring until it lands on the right owner.
- **Leaving.** `leave` ships the node's data to its successor and tells its
  successor/predecessor to point at each other, stitching the ring back
  together.

The networking layer (`ChordNode/nodeserver.cpp`) is raw BSD sockets: each
node listens on its own IP:port, and every command — `findsuccessor`,
`notify`, `upload`, `search`, `changepred`, etc. — is a plain space-separated
string sent over a fresh TCP connection.

## Repository layout

```
ChordNode/    node.cpp (entry point), nodeclass.* (ring state), nodeserver.*
              (socket listener + command handling), nodesync.* (stabilization
              background thread), util.* (hashing, socket helpers)
ChordClient/  client.cpp (entry point), clientutil.* (socket helpers)
CMakeLists.txt   build definition (see Build, below)
create_nodes.py / create_clients.py   spin up N local node/client working
                                        directories from the compiled binaries
port_map_finder.cpp   standalone dev utility — for a range of ports, computes
                       which ring ID each would hash to, useful for picking
                       ports that land on specific IDs during a demo
files/        output of port_map_finder.cpp (not read by node/client at
              runtime)
```

## Requirements

- CMake ≥ 3.16 and a C++11 compiler
- OpenSSL (headers + library) — used for SHA-1 hashing
- POSIX threads (pthreads) and BSD sockets — so Linux or macOS; this hasn't
  been tried on Windows

**macOS:** `brew install openssl@3` (plain Apple Clang works fine — no need
for a separate GCC install).
**Linux:** `sudo apt install libssl-dev build-essential` (or your
distro's equivalent).

## Build

```sh
cmake -B build
cmake --build build
```

This produces `ChordNode/node` and `ChordClient/client`. CMake finds OpenSSL
and pthreads automatically on either platform — no manual include/library
paths needed.

To rebuild from scratch:

```sh
rm -rf build
cmake -B build
cmake --build build
```

## Run

### Starting a ring

In one terminal:

```sh
./ChordNode/node
Enter Node IP
127.0.0.1
Enter Node Portno
10000
$ create_ring
```

In another terminal, join a second node to it:

```sh
./ChordNode/node
Enter Node IP
127.0.0.1
Enter Node Portno
10001
$ join 127.0.0.1 10000
```

Repeat for as many nodes as you like, each `join`-ing any existing member.

### Node commands

| Command | Effect |
|---|---|
| `create_ring` | Bootstrap a new ring (first node only) |
| `join <ip> <port>` | Join an existing ring via a known member |
| `display` | Print this node's ID, successor, and predecessor |
| `fingertable` | Print this node's finger (routing) table |
| `data_display` | Print the keys stored on this node |
| `leave` | Cleanly leave the ring, migrating data to the successor |

### Client

```sh
./ChordClient/client
Enter IP to connect
127.0.0.1
Enter Port no
10000
$ upload operatingsystem
File Uploaded to Server
$ search operatingsystem
127.0.0.1 10001
```

- `upload <string>` — hashes the string and stores it on the owning node.
- `search <string>` — returns the `ip port` of the node storing that key.
- `exit` — quit the client.

### Local multi-node demo

`create_nodes.py`/`create_clients.py` copy the compiled binaries into
`Node0/`, `Node1/`, ... and `Client0/`, `Client1/`, ... folders, so you can
open several terminals and run independent ring members/clients without them
fighting over the same working directory:

```sh
python3 create_nodes.py 4      # sets up Node0..Node3, each with its own ./node
python3 create_clients.py 2    # sets up Client0..Client1, each with its own ./client
```

(Or `cmake --build build --target script`, which runs both for you after
building.) Then `cd NodeX && ./node` in each terminal.

## Testing

There is no automated test suite — this is a small demo/learning project, and
correctness is best checked by actually running a ring:

1. Build (above), then start 2+ nodes and let them run for ~10 seconds so the
   stabilization thread converges (`display` on each should show it pointing
   at its real neighbors, not itself).
2. Use a client to `upload` a couple of strings and `search` for them —
   confirm the reported owner is consistent between runs.
3. `data_display` on each node to see which node actually holds which key.
4. `leave` a node and confirm its data reappears on its former successor via
   `data_display`.

One thing worth knowing if you're scripting against it: most commands print
immediately, but stabilization runs on a background timer, so give it a few
seconds after `join` before trusting `display`/`fingertable` output.
