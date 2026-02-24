#!/usr/bin/env python3
"""
NetAnim Cinematic 3D Visualizer
--------------------------------
Parses ns-3 NetAnim XML and renders an interactive 3D animation with:
- Node state changes (color, size, description)
- Static links with glowing effect
- Moving packet spheres (IP‑based routing)
- Live time‑series metrics subplot
- Smooth camera and controls
"""

import xml.etree.ElementTree as ET
import re
import sys
from collections import defaultdict
import numpy as np
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# -------------------------------------------------------------------------
# 1.  PARSING
# -------------------------------------------------------------------------

def parse_netanim(filepath):
    """
    Returns:
        nodes_init : dict {node_id: initial state}
        node_updates: dict {node_id: [(time, {attr})]}
        links      : list of (from_id, to_id)
        packets    : list of dict with from, to, tx, rx
    """
    tree = ET.parse(filepath)
    root = tree.getroot()

    # ---- 1.1  Node initial positions ----
    nodes_init = {}
    for node in root.findall('.//node'):
        nid = node.get('id')
        if nid is None:
            continue
        nodes_init[nid] = {
            'x': float(node.get('locX', 0)),
            'y': float(node.get('locY', 0)),
            'z': 0.0,
            'color': '#888888',
            'size': 20,
            'desc': f'Node {nid}'
        }

    # ---- 1.2  Node updates (color, size, description) ----
    raw_updates = defaultdict(list)   # (node_id, time) -> [(key, val)]
    for nu in root.findall('.//nu'):
        t = nu.get('t')
        nid = nu.get('id')
        param = nu.get('p')
        if t is None or nid is None or param is None:
            continue
        t = float(t)
        if param == 'c':   # color
            r, g, b = nu.get('r'), nu.get('g'), nu.get('b')
            if r and g and b:
                raw_updates[(nid, t)].append(('color', f'rgb({r},{g},{b})'))
        elif param == 's': # size
            w, h = nu.get('w'), nu.get('h')
            if w and h:
                sz = (float(w) + float(h)) / 2
                raw_updates[(nid, t)].append(('size', sz))
        elif param == 'd': # description
            desc = nu.get('descr', '')
            raw_updates[(nid, t)].append(('desc', desc))

    node_updates = defaultdict(list)
    for (nid, t), changes in raw_updates.items():
        state = {}
        for k, v in changes:
            state[k] = v
        node_updates[nid].append((t, state))
    for nid in node_updates:
        node_updates[nid].sort(key=lambda x: x[0])

    # ---- 1.3  IP‑to‑node mapping (from <ip n="...">) ----
    ip_to_node = {}
    for ip_elem in root.findall('.//ip'):
        nid = ip_elem.get('n')
        if nid is None:
            continue
        for addr in ip_elem.findall('address'):
            ip = addr.text.strip()
            if ip and ip != '127.0.0.1' and not ip.startswith('::'):  # ignore localhost
                ip_to_node[ip] = nid

    # ---- 1.4  Packets – extract source/destination IPs from meta‑info ----
    packets = []
    for pkt in root.findall('.//p'):
        tx_str = pkt.get('fbTx')
        rx_str = pkt.get('fbRx')
        meta = pkt.get('meta-info', '')
        if not tx_str or not rx_str or not meta:
            continue
        tx = float(tx_str)
        rx = float(rx_str)

        # Regex to find IPv4 addresses like "13.0.0.5 > 13.0.0.6"
        match = re.search(r'(\d+\.\d+\.\d+\.\d+)\s*>\s*(\d+\.\d+\.\d+\.\d+)', meta)
        if match:
            src_ip, dst_ip = match.groups()
            src_node = ip_to_node.get(src_ip)
            dst_node = ip_to_node.get(dst_ip)
            if src_node and dst_node:
                packets.append({
                    'from': src_node,
                    'to': dst_node,
                    'tx': tx,
                    'rx': rx,
                    'meta': meta[:50] + '...'
                })

    # ---- 1.5  Static links ----
    links = []
    for link in root.findall('.//link'):
        from_id = link.get('fromId')
        to_id = link.get('toId')
        if from_id and to_id:
            links.append((from_id, to_id))

    return nodes_init, node_updates, links, packets


def get_node_state(nid, time, init, updates):
    """Return node attributes at given simulation time."""
    state = init.get(nid, {}).copy()
    if nid in updates:
        for t, attrs in updates[nid]:
            if t <= time:
                state.update(attrs)
            else:
                break
    state.setdefault('x', 0)
    state.setdefault('y', 0)
    state.setdefault('z', 0)
    state.setdefault('color', '#888888')
    state.setdefault('size', 20)
    state.setdefault('desc', f'Node {nid}')
    return state


def get_all_event_times(node_updates, packets):
    """Sorted unique times from node updates and packet boundaries."""
    times = set()
    for lst in node_updates.values():
        for t, _ in lst:
            times.add(t)
    for p in packets:
        times.add(p['tx'])
        times.add(p['rx'])
    if not times:
        times.add(0.0)
    return sorted(times)

# -------------------------------------------------------------------------
# 2.  ANIMATION CONSTRUCTION
# -------------------------------------------------------------------------

def create_cinematic_animation(nodes_init, node_updates, links, packets):
    """Build Plotly figure with 3D scene + metric subplot."""
    all_nodes = set(nodes_init.keys()) | set(node_updates.keys())
    times = get_all_event_times(node_updates, packets)

    # ----- 2.1  Helper to build a node DataFrame for a given time -----
    def states_to_df(time):
        rows = []
        for nid in all_nodes:
            s = get_node_state(nid, time, nodes_init, node_updates)
            rows.append({
                'id': nid,
                'x': s['x'], 'y': s['y'], 'z': s['z'],
                'color': s['color'],
                'size': np.clip(s['size'], 5, 50),
                'desc': s['desc']
            })
        return pd.DataFrame(rows)

    # ----- 2.2  Base frame (t = first time) -----
    t0 = times[0]
    df0 = states_to_df(t0)

    # ----- 2.3  Create figure with subplots -----
    fig = make_subplots(
        rows=2, cols=1,
        row_heights=[0.7, 0.3],
        vertical_spacing=0.1,
        specs=[[{'type': 'scene'}], [{'type': 'xy'}]],
        subplot_titles=('Network Topology', 'Real‑time Metrics (simulated)')
    )

    # ----- 2.4  Static link traces (same for all frames) -----
    for from_id, to_id in links:
        if from_id in all_nodes and to_id in all_nodes:
            s_from = get_node_state(from_id, t0, nodes_init, node_updates)
            s_to   = get_node_state(to_id,   t0, nodes_init, node_updates)
            fig.add_trace(
                go.Scatter3d(
                    x=[s_from['x'], s_to['x'], None],
                    y=[s_from['y'], s_to['y'], None],
                    z=[s_from['z'], s_to['z'], None],
                    mode='lines',
                    line=dict(color='rgba(180,180,180,0.4)', width=1.5),
                    hoverinfo='none',
                    showlegend=False
                ),
                row=1, col=1
            )

    # ----- 2.5  Node trace (will be updated each frame) -----
    node_trace = go.Scatter3d(
        x=df0['x'], y=df0['y'], z=df0['z'],
        mode='markers+text',
        marker=dict(
            size=df0['size'],
            color=df0['color'],
            line=dict(width=1, color='black'),
            symbol='circle',
            opacity=0.9
        ),
        text=df0['desc'],
        hovertext=df0.apply(
            lambda r: f"<b>Node {r['id']}</b><br>{r['desc']}<br>Pos: ({r['x']:.1f}, {r['y']:.1f})",
            axis=1
        ),
        hoverinfo='text',
        name='Nodes'
    )
    fig.add_trace(node_trace, row=1, col=1)

    # ----- 2.6  Packet trace (will hold moving spheres) -----
    packet_trace = go.Scatter3d(
        x=[], y=[], z=[],
        mode='markers',
        marker=dict(size=5, color='cyan', symbol='diamond',
                    line=dict(width=0.5, color='white')),
        name='Active Packets',
        hoverinfo='text',
        text=[]
    )
    fig.add_trace(packet_trace, row=1, col=1)

    # ----- 2.7  Dummy metric trace (throughput / latency) -----
    # We'll simulate a metric based on packet count and node states
    metric_trace = go.Scatter(
        x=[], y=[],
        mode='lines',
        line=dict(color='#FF6B6B', width=2),
        name='Throughput (Mbps)'
    )
    fig.add_trace(metric_trace, row=2, col=1)

    # ----- 2.8  Prepare frames -----
    frames = []
    for time in times:
        df = states_to_df(time)
        # Node data
        node_frame = go.Scatter3d(
            x=df['x'], y=df['y'], z=df['z'],
            marker=dict(size=df['size'], color=df['color']),
            text=df['desc'],
            hovertext=df.apply(
                lambda r: f"<b>Node {r['id']}</b><br>{r['desc']}<br>Pos: ({r['x']:.1f}, {r['y']:.1f})",
                axis=1
            )
        )
        # Active packets at this time
        pkt_x, pkt_y, pkt_z, pkt_text = [], [], [], []
        for p in packets:
            if p['tx'] <= time <= p['rx']:
                s_from = get_node_state(p['from'], time, nodes_init, node_updates)
                s_to   = get_node_state(p['to'],   time, nodes_init, node_updates)
                # linear interpolation
                progress = (time - p['tx']) / (p['rx'] - p['tx'] + 1e-9)
                x = s_from['x'] + progress * (s_to['x'] - s_from['x'])
                y = s_from['y'] + progress * (s_to['y'] - s_from['y'])
                z = s_from['z'] + progress * (s_to['z'] - s_from['z'])
                pkt_x.append(x)
                pkt_y.append(y)
                pkt_z.append(z)
                pkt_text.append(f"Packet: {p['from']} → {p['to']}<br>{p['meta']}")
        packet_frame = go.Scatter3d(
            x=pkt_x, y=pkt_y, z=pkt_z,
            marker=dict(size=6, color='lime', symbol='diamond',
                        line=dict(width=0.5, color='white')),
            text=pkt_text,
            hoverinfo='text'
        )
        # Simulated metric (e.g., number of active packets * 10)
        metric_value = len(pkt_x) * 15.0 + np.random.uniform(5, 15)
        # Build full time axis for metric (cumulative)
        times_for_metric = times[:times.index(time)+1]
        values_for_metric = [len([p for p in packets if t0 <= p['rx']]) * 15.0 + 10
                             for t0 in times_for_metric]
        metric_frame = go.Scatter(
            x=times_for_metric,
            y=values_for_metric,
            mode='lines'
        )

        frames.append(go.Frame(
            data=[node_frame, packet_frame, metric_frame],
            name=f'{time:.3f}'
        ))

    fig.frames = frames

    # ----- 2.9  Layout & controls -----
    fig.update_layout(
        title='📡 5G NR + Immortal Identity – Cinematic 3D Animation',
        scene=dict(
            xaxis_title='X (m)',
            yaxis_title='Y (m)',
            zaxis_title='Z (m)',
            aspectmode='data',
            camera=dict(
                eye=dict(x=1.8, y=1.8, z=1.8),
                center=dict(x=0, y=0, z=0)
            ),
            bgcolor='rgba(10,10,20,1)'
        ),
        paper_bgcolor='rgba(10,10,20,1)',
        font=dict(color='white'),
        updatemenus=[
            dict(
                type='buttons',
                showactive=False,
                y=0.9, x=0.1,
                buttons=[
                    dict(label='▶ Play',
                         method='animate',
                         args=[None, {'frame': {'duration': 50, 'redraw': True},
                                      'fromcurrent': True}]),
                    dict(label='⏸ Pause',
                         method='animate',
                         args=[[None], {'frame': {'duration': 0, 'redraw': False},
                                        'mode': 'immediate'}])
                ]
            )
        ],
        sliders=[
            dict(
                steps=[
                    dict(
                        method='animate',
                        args=[[f.name], {'mode': 'immediate',
                                         'frame': {'duration': 0, 'redraw': True}}],
                        label=f'{float(f.name):.1f}'
                    ) for f in frames
                ],
                active=0,
                currentvalue=dict(
                    prefix='⏱ Time: ',
                    font=dict(size=14, color='white')
                ),
                len=0.9,
                x=0.05,
                pad=dict(b=10, t=10)
            )
        ]
    )

    # Style metric subplot
    fig.update_xaxes(title_text='Time (s)', row=2, col=1, color='white')
    fig.update_yaxes(title_text='Throughput (Mbps)', row=2, col=1, color='white')
    fig.update_traces(row=2, col=1)

    return fig

# -------------------------------------------------------------------------
# 3.  MAIN
# -------------------------------------------------------------------------

def main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} <NetAnim XML file>')
        sys.exit(1)

    filepath = sys.argv[1]
    print(f'⚙ Parsing {filepath}...')
    nodes_init, node_updates, links, packets = parse_netanim(filepath)

    print(f'   • {len(nodes_init)} nodes, {len(node_updates)} updated nodes')
    print(f'   • {len(links)} static links')
    print(f'   • {len(packets)} routable packets')

    fig = create_cinematic_animation(nodes_init, node_updates, links, packets)

    out = filepath.replace('.xml', '_cinematic.html')
    fig.write_html(out, include_plotlyjs='cdn', full_html=True,
                   config={'displaylogo': False, 'modeBarButtonsToRemove': ['lasso2d', 'select2d']})
    print(f'✅ Saved: {out}')
    fig.show()

if __name__ == '__main__':
    main()
