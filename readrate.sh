#!/usr/bin/env bash

# Default parameters
ENDPOINT="192.168.1.104:4840"
NODE_ID="ns=1;s=ADC.Channel0"
NUM_CLIENTS=4
VERBOSE=0

# Parse CLI flags
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -c|--clients)
            NUM_CLIENTS="$2"
            shift 2
            ;;
        -e|--endpoint)
            ENDPOINT="$2"
            shift 2
            ;;
        -n|--node)
            NODE_ID="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: ./readrate.sh [options]"
            echo "Options:"
            echo "  -c, --clients <num>    Number of concurrent parallel clients (default: 4)"
            echo "  -v, --verbose          Enable verbose response output from workers"
            echo "  -e, --endpoint <url>   OPC UA endpoint IP:Port (default: 192.168.1.104:4840)"
            echo "  -n, --node <node_id>   Target Node ID (default: ns=1;s=ADC.Channel0)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Strip opc.tcp:// prefix if user passed it, preventing duplicate prefix in UI
ENDPOINT="${ENDPOINT#opc.tcp://}"

# Set up temporary runtime directory for IPC stats
TMP_DIR=$(mktemp -d)
STATS_FILE="$TMP_DIR/stats.log"
touch "$STATS_FILE"

# Clean signal handling for background threads and terminal state
cleanup() {
    pkill -P $$ 2>/dev/null
    rm -rf "$TMP_DIR"
    tput cnorm 2>/dev/null
    stty sane 2>/dev/null
    echo -e "\n\nBenchmark stopped."
    exit 0
}
trap cleanup INT TERM EXIT

# Worker function executed by each parallel background client
run_worker() {
    local worker_id=$1
    while true; do
        start_ms=$(date +%s%3N)
        raw_out=$(./opcua-cli read -t=10 -s=None "$ENDPOINT" "$NODE_ID" 2>&1)
        ret_code=$?
        end_ms=$(date +%s%3N)
        latency=$((end_ms - start_ms))

        if [ $ret_code -eq 0 ] && [[ "$raw_out" != *"Error"* ]] && [[ "$raw_out" != *"Connection failed"* ]]; then
            echo "OK $latency" >> "$STATS_FILE"
            if [ $VERBOSE -eq 1 ]; then
                clean_out=$(echo "$raw_out" | tr '\r\n' ' ')
                printf "[Client #%d] [%4d ms] %s\n" "$worker_id" "$latency" "$clean_out"
            fi
        else
            echo "ERR $latency" >> "$STATS_FILE"
            if [ $VERBOSE -eq 1 ]; then
                clean_err=$(echo "$raw_out" | tr '\r\n' ' ')
                printf "[Client #%d] [ ERROR ] %s\n" "$worker_id" "$clean_err"
            fi
        fi
    done
}

# ASCII Gauge generator
draw_gauge() {
    local val=${1:-0}
    local max=50          # Full gauge scale at 50 req/sec
    local width=20        # Width of visual bar
    
    # Ensure val is a valid integer
    val=$((val + 0)) 2>/dev/null || val=0

    local filled=$(( val * width / max ))
    [ $filled -gt $width ] && filled=$width
    [ $filled -lt 0 ] && filled=0
    local empty=$(( width - filled ))

    local bar=""
    for ((i=0; i<filled; i++)); do bar+="█"; done
    for ((i=0; i<empty; i++)); do bar+="░"; done
    echo "[$bar]"
}

# Launch parallel worker threads
echo "Spawning $NUM_CLIENTS parallel OPC UA client workers..."
for ((i=1; i<=NUM_CLIENTS; i++)); do
    run_worker "$i" &
done

# If verbose mode is enabled, stream worker output
if [ $VERBOSE -eq 1 ]; then
    echo "Running in Verbose Mode. Press Ctrl+C to stop..."
    echo "----------------------------------------------------------------------"
    wait
fi

# Default Dashboard Mode
tput civis 2>/dev/null
clear

# Make terminal stdin non-blocking for 'q' key exit
stty -icanon -echo time 0 min 0 2>/dev/null

last_count=0
last_time=$(date +%s%3N)

while true; do
    sleep 0.3

    now_time=$(date +%s%3N)
    elapsed_ms=$((now_time - last_time))
    
    total_reqs=$(wc -l < "$STATS_FILE" 2>/dev/null || echo 0)
    total_reqs=${total_reqs//[[:space:]]/}
    [ -z "$total_reqs" ] && total_reqs=0

    # Fixed grep evaluation: capture output safely without triggering duplicate echo
    err_count=$(grep -c "^ERR" "$STATS_FILE" 2>/dev/null)
    err_count=${err_count//[[:space:]]/}
    [ -z "$err_count" ] && err_count=0

    ok_count=$((total_reqs - err_count))

    # Calculate real-time throughput rate (req/sec)
    new_reqs=$((total_reqs - last_count))
    if [ $elapsed_ms -gt 0 ]; then
        req_per_sec=$(awk -v new="$new_reqs" -v ms="$elapsed_ms" 'BEGIN { printf "%.1f", (new * 1000) / ms }')
    else
        req_per_sec="0.0"
    fi

    last_count=$total_reqs
    last_time=$now_time

    # Calculate moving average latency of the last 30 requests
    avg_lat=$(tail -n 30 "$STATS_FILE" 2>/dev/null | awk '{sum+=$2; count++} END {if(count>0) printf "%d", sum/count; else print "0"}')

    # Convert floating point to integer for gauge scaling
    hz_int=${req_per_sec%.*}
    hz_int=${hz_int//[[:space:]]/}
    [ -z "$hz_int" ] && hz_int=0

    gauge=$(draw_gauge "$hz_int")

    # Draw dashboard using tput position reset to prevent flicker
    tput cup 0 0 2>/dev/null
    echo "======================================================================"
    echo "            OPC UA Multi-Client Parallel Benchmark Tool               "
    echo "======================================================================"
    printf " Target Endpoint : opc.tcp://%-35s\n" "$ENDPOINT"
    printf " Target Node ID  : %-40s\n" "$NODE_ID"
    printf " Active Clients  : %-2d parallel workers\n" "$NUM_CLIENTS"
    echo "----------------------------------------------------------------------"
    printf " Throughput      : %s  %5s req/sec\n" "$gauge" "$req_per_sec"
    printf " Avg Latency     : %4d ms / read\n" "$avg_lat"
    printf " Total Requests  : %-6d (OK: %-5d | ERR: %-5d)\n" "$total_reqs" "$ok_count" "$err_count"
    echo "----------------------------------------------------------------------"
    echo " Press [Q] or Ctrl+C to stop..."

    # Check for non-blocking 'q' keypress to quit
    if read -t 0.001 -n 1 key 2>/dev/null; then
        if [[ "$key" == "q" || "$key" == "Q" ]]; then
            break
        fi
    fi
done
