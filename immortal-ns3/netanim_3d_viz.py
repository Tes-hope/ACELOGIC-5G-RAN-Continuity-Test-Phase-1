#!/usr/bin/env python3
"""
3D Visualizer for ns-3 NetAnim XML – works with your actual file.
Parses node positions, updates (color/size/desc), and links.
"""

import xml.etree.ElementTree as ET
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from collections import defaultdict
import sys

# -----------------------------------------------------------------------------
#  Parser
# -----------------------------------------------------------------------------

def parse_netanim_xml(filepath):
    """
    Returns:
        nodes_init: dict {node_id: {'x','y','z','color','size','desc'}} at t=0
        node_updates: dict {node_id: list of (time, {attr})}
        links: list of (from_id, to_id)
    """
    tree = ET.parse(filepath)
    root = tree.getroot()

    # --- 1. Initial node positions from <node> tags ---
    nodes_init = {}
    for node in root.findall('.//node'):
        nid = node.get('id')
        if nid is None:
            continue
        x = float(node.get('locX', 0))
        y = float(node.get('locY', 0))
        z = 0.0  # no Z in file, set to 0
        nodes_init[nid] = {
            'x': x, 'y': y, 'z': z,
            'color': '#888888',  # default gray
            'size': 20,          # default size
            'desc': f'Node {nid}'
        }

    # --- 2. Updates from <nu> tags ---
    # We'll collect all updates per node, then later merge by time
    raw_updates = defaultdict(list)  # key: (node_id, time)

    for nu in root.findall('.//nu'):
        t = nu.get('t')
        nid = nu.get('id')
        param = nu.get('p')
        if t is None or nid is None or param is None:
            continue
        t = float(t)
        # color update
        if param == 'c':
            r = nu.get('r')
            g = nu.get('g')
            b = nu.get('b')
            if r and g and b:
                color = f'rgb({r},{g},{b})'
                raw_updates[(nid, t)].append(('color', color))
        # size update
        elif param == 's':
            w = nu.get('w')
            h = nu.get('h')
            if w and h:
                # use average of w and h as marker size
                size = (float(w) + float(h)) / 2
                raw_updates[(nid, t)].append(('size', size))
        # description update
        elif param == 'd':
            desc = nu.get('descr', '')
            raw_updates[(nid, t)].append(('desc', desc))

    # Merge updates for same node and time into one dict
    node_updates = defaultdict(list)
    for (nid, t), changes in raw_updates.items():
        state = {}
        for key, val in changes:
            state[key] = val
        node_updates[nid].append((t, state))

    # Sort updates for each node by time
    for nid in node_updates:
        node_updates[nid].sort(key=lambda x: x[0])

    # --- 3. Static links ---
    links = []
    for link in root.findall('.//link'):
        from_id = link.get('fromId')
        to_id = link.get('toId')
        if from_id and to_id:
            links.append((from_id, to_id))

    return nodes_init, node_updates, links


def get_node_state_at_time(node_id, time, nodes_init, node_updates):
    """Return node attributes at given simulation time."""
    state = nodes_init.get(node_id, {}).copy()
    if node_id in node_updates:
        for t, attrs in node_updates[node_id]:
            if t <= time:
                state.update(attrs)
            else:
                break
    # Ensure required keys exist
    state.setdefault('x', 0)
    state.setdefault('y', 0)
    state.setdefault('z', 0)
    state.setdefault('color', '#888888')
    state.setdefault('size', 20)
    state.setdefault('desc', f'Node {node_id}')
    return state


def get_all_event_times(node_updates):
    """Return sorted list of all distinct times where any update occurs."""
    times = set()
    for updates in node_updates.values():
        for t, _ in updates:
            times.add(t)
    return sorted(times)

# -----------------------------------------------------------------------------
#  Plotting
# -----------------------------------------------------------------------------

def create_animated_3d_plot(nodes_init, node_updates, links):
    """Create a Plotly 3D animation with frames for each event time."""

    all_node_ids = set(nodes_init.keys()).union(set(node_updates.keys()))
    times = get_all_event_times(node_updates)
    if not times:
        # if no updates, show initial state at t=0
        times = [0.0]

    # --- Build base frame (t = first time) ---
    first_time = times[0]
    base_states = {}
    for nid in all_node_ids:
        base_states[nid] = get_node_state_at_time(nid, first_time, nodes_init, node_updates)

    # Convert to DataFrame for plotting
    def states_to_df(states_dict):
        rows = []
        for nid, attrs in states_dict.items():
            rows.append({
                'id': nid,
                'x': attrs['x'],
                'y': attrs['y'],
                'z': attrs['z'],
                'color': attrs['color'],
                'size': attrs['size'],
                'desc': attrs['desc']
            })
        df = pd.DataFrame(rows)
        if not df.empty:
            df['marker_size'] = df['size'].clip(lower=5, upper=50)
        return df

    df_base = states_to_df(base_states)

    # --- Create figure ---
    fig = go.Figure()

    # Add static links as lines (will be same for all frames)
    for from_id, to_id in links:
        if from_id in base_states and to_id in base_states:
            x0, y0, z0 = base_states[from_id]['x'], base_states[from_id]['y'], base_states[from_id]['z']
            x1, y1, z1 = base_states[to_id]['x'], base_states[to_id]['y'], base_states[to_id]['z']
            fig.add_trace(go.Scatter3d(
                x=[x0, x1, None],
                y=[y0, y1, None],
                z=[z0, z1, None],
                mode='lines',
                line=dict(color='rgba(100,100,100,0.3)', width=1),
                hoverinfo='none',
                showlegend=False
            ))

    # Add node trace (will be updated per frame)
    fig.add_trace(go.Scatter3d(
        x=df_base['x'] if not df_base.empty else [],
        y=df_base['y'] if not df_base.empty else [],
        z=df_base['z'] if not df_base.empty else [],
        mode='markers+text',
        marker=dict(
            size=df_base['marker_size'] if not df_base.empty else [],
            color=df_base['color'] if not df_base.empty else [],
            line=dict(width=1, color='black'),
            symbol='circle'
        ),
        text=df_base['desc'] if not df_base.empty else [],
        hovertext=df_base.apply(
            lambda row: f"<b>Node {row['id']}</b><br>{row['desc']}<br>Pos: ({row['x']:.1f}, {row['y']:.1f}, {row['z']:.1f})",
            axis=1
        ) if not df_base.empty else [],
        hoverinfo='text',
        name='Nodes'
    ))

    # --- Build frames for each distinct time ---
    frames = []
    for t in times:
        states = {}
        for nid in all_node_ids:
            states[nid] = get_node_state_at_time(nid, t, nodes_init, node_updates)
        df = states_to_df(states)
        frame_data = [
            go.Scatter3d(
                x=df['x'], y=df['y'], z=df['z'],
                mode='markers+text',
                marker=dict(
                    size=df['marker_size'],
                    color=df['color'],
                    line=dict(width=1, color='black'),
                    symbol='circle'
                ),
                text=df['desc'],
                hovertext=df.apply(
                    lambda row: f"<b>Node {row['id']}</b><br>{row['desc']}<br>Pos: ({row['x']:.1f}, {row['y']:.1f}, {row['z']:.1f})",
                    axis=1
                ),
                hoverinfo='text',
                name='Nodes'
            )
        ]
        frames.append(go.Frame(data=frame_data, name=f'{t:.2f}'))

    fig.frames = frames

    # --- Slider and play button ---
    sliders = [dict(
        steps=[dict(
            method='animate',
            args=[[f.name], dict(mode='immediate', frame=dict(duration=0, redraw=True))],
            label=f.name
        ) for f in frames],
        active=0,
        x=0.1, y=0,
        len=0.9,
        currentvalue=dict(
            prefix='Time: ',
            visible=True,
            xanchor='center'
        ),
        pad=dict(b=10, t=10),
    )]

    updatemenus = [dict(
        type='buttons',
        showactive=False,
        buttons=[
            dict(label='Play',
                 method='animate',
                 args=[None, dict(frame=dict(duration=50, redraw=True),
                                  fromcurrent=True,
                                  mode='immediate')]),
            dict(label='Pause',
                 method='animate',
                 args=[[None], dict(frame=dict(duration=0, redraw=False),
                                    mode='immediate')])
        ],
        x=0.1, y=0,
        xanchor='right',
        yanchor='top',
        pad=dict(r=10, t=10),
    )]

    fig.update_layout(
        title='NetAnim 3D Animation',
        scene=dict(
            xaxis_title='X',
            yaxis_title='Y',
            zaxis_title='Z',
            aspectmode='data',
            camera=dict(eye=dict(x=1.5, y=1.5, z=1.5))
        ),
        updatemenus=updatemenus,
        sliders=sliders,
        margin=dict(l=0, r=0, b=40, t=40)
    )

    return fig

# -----------------------------------------------------------------------------
#  Main
# -----------------------------------------------------------------------------

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <NetAnim XML file>")
        sys.exit(1)

    filepath = sys.argv[1]
    print(f"Parsing {filepath} ...")
    nodes_init, node_updates, links = parse_netanim_xml(filepath)

    print(f"Found {len(nodes_init)} nodes, {sum(len(v) for v in node_updates.values())} updates, {len(links)} links.")

    if not nodes_init:
        print("No nodes found. Exiting.")
        sys.exit(1)

    fig = create_animated_3d_plot(nodes_init, node_updates, links)

    out_html = filepath.replace('.xml', '_3d.html')
    fig.write_html(out_html)
    print(f"Saved interactive 3D animation to: {out_html}")
    fig.show()


if __name__ == '__main__':
    main()
