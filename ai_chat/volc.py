import asyncio
import base64
import json
import wave
import numpy as np
import soundfile as sf
from scipy.signal import resample
import websockets


def resample_audio(audio_data, original_sample_rate, target_sample_rate):
    number_of_samples = round(
        len(audio_data) * float(target_sample_rate) / original_sample_rate)
    resampled_audio = resample(audio_data, number_of_samples)
    return resampled_audio.astype(np.int16)


def pcm_to_wav(pcm_data, wav_file, sample_rate=16000, num_channels=1, sample_width=2):
    print(f"saved to file {wav_file}")
    with wave.open(wav_file, 'wb') as wav:
        # Number of channels (1 for mono, 2 for stereo)
        wav.setnchannels(num_channels)
        # Sample width in bytes (2 for 16-bit audio)
        wav.setsampwidth(sample_width)
        wav.setframerate(sample_rate)
        wav.writeframes(pcm_data)


async def send_audio(client, audio_file_path: str):
    sample_rate = 16000
    duration_ms = 100
    samples_per_chunk = sample_rate * (duration_ms / 1000)
    bytes_per_sample = 2
    bytes_per_chunk = int(samples_per_chunk * bytes_per_sample)

    audio_data, original_sample_rate = sf.read(
        audio_file_path, dtype="int16")

    if original_sample_rate != sample_rate:
        audio_data = resample_audio(
            audio_data, original_sample_rate, sample_rate)

    audio_bytes = audio_data.tobytes()

    print('Sending: "打开灯"音频')
    for i in range(0, len(audio_bytes), bytes_per_chunk):
        chunk = audio_bytes[i: i + bytes_per_chunk]
        base64_audio = base64.b64encode(chunk).decode("utf-8")
        append_event = {
            "type": "input_audio_buffer.append",
            "audio": base64_audio
        }
        await client.send(json.dumps(append_event, ensure_ascii=False))

    commit_event = {
        "type": "input_audio_buffer.commit"
    }
    print("Sending:\n{\n    \"type\": \"input_audio_buffer.commit\"\n}")
    await client.send(json.dumps(commit_event, ensure_ascii=False))

    event = {
        "type": "response.create",
        "response": {
            "modalities": ["text", "audio"]
        }
    }
    print("Sending:\n{\n    \"type\": \"response.create\",\n    \"response\": { \n        \"modalities\": [\"text\", \"audio\"]\n    }\n}")
    await client.send(json.dumps(event, ensure_ascii=False))


def _tool_schemas():
    return [
        {
            "type": "function",
            "name": "turn_on_light",
            "description": "打开指定房间或设备的灯",
            "parameters": {
                "type": "object",
                "properties": {
                    "room": {"type": "string", "description": "房间名，如客厅/卧室"}
                },
                "required": []
            }
        }
    ]


def get_session_update_msg():
    config = {
        "modalities": ["text", "audio"],
        "instructions": "你的名字叫豆包，你是一个智能助手",
        "voice": "zh_female_tianmeixiaoyuan_moon_bigtts",
        "input_audio_format": "pcm16",
        "output_audio_format": "pcm16",
        "tool_choice": "auto",
        "turn_detection": None,
        "temperature": 0.8,
        "tools": _tool_schemas(),
    }
    event = {
        "type": "session.update",
        "session": config
    }
    return json.dumps(event, ensure_ascii=False)


async def _execute_tool_and_respond(client, name: str, args: dict, call_id: str):
    if name == "turn_on_light":
        result = {"result": "打开成功"}
    else:
        result = {"error": "unknown tool: {name}", "args": args}

    tool_output_event = {
        "type": "conversation.item.create",
        "item": {
            "type": "function_call_output",
            "call_id": call_id,
            "output": json.dumps(result, ensure_ascii=False),
        }
    }
    print("Sending:\n" + json.dumps(tool_output_event, ensure_ascii=False))
    await client.send(json.dumps(tool_output_event, ensure_ascii=False))

    await asyncio.sleep(2)
    follow_response = {
        "type": "response.create",
        "response": {"modalities": ["text", "audio"]}
    }
    print("Sending:\n" + json.dumps(follow_response, ensure_ascii=False))
    await client.send(json.dumps(follow_response, ensure_ascii=False))


async def receive_messages(client, save_file_name):
    audio_list = bytearray()
    response_index = 0
    func_call_state = None  # {call_id, name, args_str}

    while not client.closed:
        message = await client.recv()
        if message is None:
            continue
        event = json.loads(message)
        message_type = event.get("type")

        if message_type == "response.audio.delta":
            audio_bytes = base64.b64decode(event["delta"])
            audio_list.extend(audio_bytes)
            continue

        if message_type in ("response.created", "session.updated", "input_audio_buffer.committed"):
            print(f"Received \"type\":\"{message_type}\"")
            continue

        if message_type == "response.output_item.added":
            item = event.get("item") or event.get("output_item") or {}
            if isinstance(item, dict) and item.get("type") == "function_call":
                func_call_state = {
                    "call_id": item.get("call_id"),
                    "name": item.get("name"),
                    "args_str": "",
                }
            sample_rate = 16000
            duration_ms = 100
            samples_per_chunk = sample_rate * (duration_ms / 1000)
            bytes_per_sample = 2
            bytes_per_chunk = int(samples_per_chunk * bytes_per_sample)

            audio_data, original_sample_rate = sf.read(
                audio_file_path, dtype="int16")

            if original_sample_rate != sample_rate:
                audio_data = resample_audio(
                    audio_data, original_sample_rate, sample_rate)

            audio_bytes = audio_data.tobytes()

            print('Sending: "打开灯"音频')
            for i in range(0, 1, bytes_per_chunk):
                chunk = audio_bytes[i: i + bytes_per_chunk]
                base64_audio = base64.b64encode(chunk).decode("utf-8")
                append_event = {
                    "type": "input_audio_buffer.append",
                    "audio": base64_audio
                }
                await client.send(json.dumps(append_event, ensure_ascii=False))

            continue

        if message_type == "response.function_call_arguments.delta":
            if func_call_state is not None:
                func_call_state["args_str"] += event.get("delta", "")
            continue

        if message_type == "response.function_call_arguments.done":
            continue

        if message_type == 'response.done':
            if func_call_state is not None:
                try:
                    args = json.loads(func_call_state["args_str"]) if func_call_state["args_str"] else {}
                except Exception:
                    args = {}
                await asyncio.sleep(2)
                await _execute_tool_and_respond(
                    client,
                    func_call_state.get("name") or "",
                    args,
                    func_call_state.get("call_id") or "",
                )
                func_call_state = None
            if audio_list:
                # 保存每个response的音频，第一段用原名，后续加序号
                fname = save_file_name
                if response_index > 0:
                    if "." in save_file_name:
                        base, ext = save_file_name.rsplit(".", 1)
                        fname = f"{base}_{response_index}.{ext}"
                    else:
                        fname = f"{save_file_name}_{response_index}"
                pcm_to_wav(audio_list, fname)
                audio_list = bytearray()
            print("Received \"type\":\"response.done\"")
            response_index += 1
            # 这里示例期望两轮：第一轮触发函数调用，第二轮播放最终反馈
            # if response_index >= 2:
            #     break
            continue

        print(event)
        continue


async def with_realtime(audio_file_path: str, save_file_name: str):
    ws_url = "wss://ai-gateway.vei.volces.com/v1/realtime?model=AG-voice-chat-agent"
    key = "your key"
    headers = {
        "Authorization": f"Bearer {key}",
    }
    async with websockets.connect(ws_url, extra_headers=headers) as client:
        session_msg = get_session_update_msg()
        await client.send(session_msg)
        print('Received "type":"session.updated"')
        await asyncio.gather(send_audio(client, audio_file_path),
                             receive_messages(client, save_file_name))
        await asyncio.sleep(0.5)


if __name__ == "__main__":
    audio_file_path = "1763209072623.wav"  # 下载示例音频
    # audio_file_path = "demo_audio_nihaoya.wav"
    save_file_name = "response_demo.wav"
    asyncio.run(with_realtime(audio_file_path, save_file_name))
