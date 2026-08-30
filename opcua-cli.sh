#!/usr/bin/env python3
import sys
import asyncio
from asyncua import Client

async def main():
    # Simple argument parser for: opcua-cli read [options] endpoint node_id
    args = sys.argv[1:]
    if not args or args[0] != "read":
        print("Error: only 'read' command is supported by this wrapper.")
        sys.exit(1)
        
    endpoint = None
    node_id = None
    
    for arg in args[1:]:
        if arg.startswith("-"):
            continue # ignore timeout/security flags for the wrapper
        elif not endpoint:
            endpoint = arg
        elif not node_id:
            node_id = arg
            
    if not endpoint or not node_id:
        print("Error: missing endpoint or node id")
        sys.exit(1)
        
    if not endpoint.startswith("opc.tcp://"):
        endpoint = f"opc.tcp://{endpoint}"
        
    try:
        async with Client(url=endpoint, timeout=3) as client:
            node = client.get_node(node_id)
            val = await node.get_value()
            print(val)
    except Exception as e:
        print(f"Connection failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    asyncio.run(main())