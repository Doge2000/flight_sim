from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.staticfiles import StaticFiles
import subprocess, asyncio, threading

app = FastAPI()


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    loop = asyncio.get_event_loop()
    process = None
    try:
        config = await websocket.receive_json()
        args = ["sim.exe"] + [str(a) for a in config["args"]]
        process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        while True:
            line = await loop.run_in_executor(None, process.stdout.readline)
            if not line:
                break
            text = line.decode().strip()
            if not text: continue
            await websocket.send_text(text)
  
    except WebSocketDisconnect:
        if process and process.poll() is None:
            process.terminate()

    



app.mount("/", StaticFiles(directory=".", html=True), name="static")

