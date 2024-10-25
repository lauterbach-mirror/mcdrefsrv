# MCD Client Stub

The MCD Client Stub is a shared library which implements the [MCD API](https://www.lauterbach.com/products/software/debugger-for-simulators/mcd-api)
and which can be linked by MCD clients such as [TRACE32](https://www.lauterbach.com/products/software/debugger-for-simulators).

## Communication between Client and Server

The main responsibility of the client stub is to establish a communication channel between the client and the server.
API calls by the client are converted into requests, sent to the server, and any response from the server is then returned back to the client.

```text
Client                              Server
     |                               |
     | --------- Request -------->   |
     | <------- [Response] -------   |
     |                               |
```

The client stub supports a custom serial protocol layer which is defined in `mcd_rpc.h` with the intention to be kept as thin as possible.

**Request Packet**:

```text
      1 Byte               0 - 64KiB
[ MCD Function ID ] [ Marshalled Arguments ]
```

**Response Packet**:

```text
         0 - 64KiB
[ Marshalled Return Values ]
```

## Adapter between Client and Server

Even when client and server are able to communicate, there are cases in which a plain transmission of data is not sufficient:

- The client expects different names for memory spaces.
- The client expects different names for registers.
- The client requires registers which are unknown to the server but which can be mimicked by the provided registers.

As an adapter, the client stub provides the functionality to convert the requests from the client such that the server understands them, and vice versa.

```text
Client                              Server
      |                             |
      |      -----------------      |
      |     | Memory Spaces   |     |
      | <---| Register Groups |---- |
      |     | Registers       |     |
      |      -----------------      |
      |      -----------------      |
      | ----| Transactions    |---> |
      |      -----------------      |
 ```

## How to Build the Client Stub

```cmd
mkdir <directory>
cmake -B <directory>
cmake --build <directory> [--config Debug]
```
