#!/usr/bin/env python3
import argparse
import threading
import struct
import time
from collections import deque
import os

import serial
import msgpack
import json

import matplotlib

# Better headless detection that works with Wayland
headless = False

# Check for display environment variables (X11 or Wayland)
if not (os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY')):
    headless = True

if headless:
    matplotlib.use('Agg')
else:
    # Use Qt5Agg for better Wayland support (falls back to TkAgg if Qt not available)
    try:
        matplotlib.use('Qt5Agg')
    except Exception:
        try:
            matplotlib.use('QtAgg')  # Qt6
        except Exception:
            pass  # Let matplotlib choose default

import matplotlib.pyplot as plt
import matplotlib.animation as animation


class SerialReader(threading.Thread):
    def __init__(self, port, baud, queues):
        super().__init__(daemon=True)
        self.ser = serial.Serial(port, baud, timeout=1)
        self.queues = queues
        self.running = True

    def run(self):
        while self.running:
            hdr = self.ser.read(1)
            if not hdr:
                continue
            fmt = hdr[0]
            if fmt == ord('M'):
                # read 4-byte length
                ln = self.ser.read(4)
                if len(ln) < 4:
                    continue
                (length,) = struct.unpack('<I', ln)
                payload = self._read_exact(length)
                if payload is None:
                    continue
                try:
                    obj = msgpack.unpackb(payload, raw=False)
                except Exception:
                    continue
            elif fmt == ord('J'):
                ln = self.ser.read(4)
                if len(ln) < 4:
                    continue
                (length,) = struct.unpack('<I', ln)
                payload = self._read_exact(length)
                if payload is None:
                    continue
                try:
                    obj = json.loads(payload.decode('utf-8'))
                except Exception:
                    continue
            else:
                # Unknown format; try to resync by reading a line
                _ = self.ser.readline()
                continue

            # Expect object with 't' and 'w' array
            try:
                ts = obj.get('t', int(time.time() * 1000))
                w = obj.get('w', [])
            except Exception:
                continue

            # push into queues
            for i, q in enumerate(self.queues):
                if i < len(w):
                    q.append((ts, float(w[i])))

    def _read_exact(self, n):
        buf = bytearray()
        while len(buf) < n:
            chunk = self.ser.read(n - len(buf))
            if not chunk:
                return None
            buf.extend(chunk)
        return bytes(buf)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', '-p', default='/dev/ttyUSB0')
    parser.add_argument('--baud', '-b', type=int, default=115200)
    parser.add_argument('--samples', '-s', type=int, default=400)
    args = parser.parse_args()

    maxlen = args.samples
    queues = [deque(maxlen=maxlen) for _ in range(4)]

    reader = SerialReader(args.port, args.baud, queues)
    reader.start()

    # Prefer seaborn-darkgrid when available, otherwise fall back gracefully
    try:
        plt.style.use('seaborn-darkgrid')
    except Exception:
        try:
            plt.style.use('seaborn-v0_8-darkgrid')
        except Exception:
            try:
                plt.style.use('seaborn')
            except Exception:
                plt.style.use('ggplot')

    fig, ax = plt.subplots()
    lines = [ax.plot([], [], label=f'Scale {i}')[0] for i in range(4)]
    ax.legend(loc='upper right')
    ax.set_xlabel('Time (s)')
    ax.set_ylabel('Weight (units)')

    def init():
        ax.set_xlim(0, 10)
        ax.set_ylim(-10, 1100)
        return lines

    def update(frame):
        # convert queues to arrays and set data
        all_ts = [[(t/1000.0) for (t, v) in q] for q in queues]
        all_vals = [[v for (t, v) in q] for q in queues]

        # set x-limits based on newest timestamp
        max_t = 0
        for ts in all_ts:
            if ts:
                max_t = max(max_t, ts[-1])
        if max_t > 0:
            ax.set_xlim(max(0, max_t - 10), max_t)

        for i, line in enumerate(lines):
            line.set_data(all_ts[i], all_vals[i])

        return lines

    if headless:
        out_path = os.path.join(os.path.dirname(__file__) or '.', 'scale_plot.png')
        print(f'Headless mode: saving live plot to {out_path}')
        try:
            while True:
                update(None)
                fig.savefig(out_path)
                time.sleep(0.5)
        except KeyboardInterrupt:
            print('Exiting headless plotter')
    else:
        ani = animation.FuncAnimation(fig, update, init_func=init, interval=100, blit=False)
        plt.show()


if __name__ == '__main__':
    main()