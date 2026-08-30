#!/usr/bin/env python3
"""
MECHA-WHISPERER | Local Web Dashboard Server & USB Bridge
Serves the luxury web dashboard on http://localhost:8000
"""

import http.server
import socketserver
import os
import sys
import webbrowser

PORT = 8000
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

def run():
    os.chdir(DIRECTORY)
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        url = f"http://localhost:{PORT}"
        print("=" * 60)
        print("  MECHA-WHISPERER: The Stethoscope for Machines")
        print(f"  Web Dashboard running at: {url}")
        print("  Press Ctrl+C to stop server")
        print("=" * 60)
        
        # Auto open browser
        try:
            webbrowser.open(url)
        except Exception:
            pass
            
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")
            httpd.server_close()

if __name__ == "__main__":
    run()
