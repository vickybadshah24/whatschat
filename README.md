# WhatsApp AI Bot — C++ Engine + Node.js Bridge

```
WhatsApp  ←→  Node.js Bridge  ←→  C++ AI Engine  ←→  Claude API
             (whatsapp-web.js)     (your code!)
```

The **C++ engine** handles all AI logic: contact filtering, conversation memory, and Claude API calls.
The **Node.js bridge** is a thin connector — it only handles the WhatsApp protocol.

---

## Requirements

| Tool | Version | Install |
|---|---|---|
| **Node.js** | 18+ | https://nodejs.org |
| **Google Chrome** | latest | https://google.com/chrome |
| **CMake** | 3.16+ | `sudo apt install cmake` / brew |
| **libcurl (dev)** | any | `sudo apt install libcurl4-openssl-dev` |
| **C++ compiler** | C++17 | `sudo apt install g++` / Xcode CLT |
| **Anthropic API Key** | — | https://console.anthropic.com |

---

## Setup

### Step 1 — Build the C++ Engine

```bash
cd cpp-engine

# Create nlohmann directory and download json.hpp header
mkdir -p nlohmann
curl -L https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp \
     -o nlohmann/json.hpp

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

This creates `cpp-engine/build/whatsapp_engine`.

### Step 2 — Edit config.json

Open `cpp-engine/config.json`:

```json
{
  "persona": {
    "name": "Rahul",
    "description": "You are Rahul, a marketing manager from Mumbai. Friendly, concise, replies in Hinglish when needed."
  },
  "allowed_contacts": [
    "919876543210@c.us",
    "919000000001@c.us"
  ],
  "anthropic_api_key": "sk-ant-xxxxxxxx"
}
```

> **Contact ID format**: `[country code][number]@c.us`
> India +91 98765 43210 → `919876543210@c.us`
> USA   +1  415 555 0100 → `14155550100@c.us`
>
> **Tip**: Leave `allowed_contacts` as `[]` to reply to **everyone**.
>
> **Tip**: Set `ANTHROPIC_API_KEY` as an environment variable instead of putting it in the JSON file.

### Step 3 — Install Node.js Bridge

```bash
cd node-bridge
npm install
```

### Step 4 — Run Both Processes

Open **two terminals**:

**Terminal 1 — C++ Engine:**
```bash
cd cpp-engine
./build/whatsapp_engine
# Output: [Engine] Listening on port 8765...
```

**Terminal 2 — Node.js Bridge:**
```bash
cd node-bridge
npm start
# Shows QR code → scan with WhatsApp
```

Scan the QR, and the bot is live! 🟢

---

## Project Structure

```
whatsapp-cpp-bot/
├── cpp-engine/
│   ├── main.cpp            ← Entry point, request loop
│   ├── config.h / .cpp     ← Loads config.json
│   ├── contact_filter.h/.cpp ← Whitelist logic
│   ├── chat_history.h/.cpp ← Per-contact memory
│   ├── claude_api.h/.cpp   ← Calls Anthropic API via libcurl
│   ├── http_server.h/.cpp  ← Listens for requests from bridge
│   ├── CMakeLists.txt      ← Build system
│   └── config.json         ← ✏️  YOUR SETTINGS GO HERE
└── node-bridge/
    ├── bridge.js           ← WhatsApp connector
    └── package.json
```

---

## Configuration Reference

| Key | Default | Description |
|---|---|---|
| `persona.name` | `"Alex"` | Your name |
| `persona.description` | See file | Instructions for the AI |
| `allowed_contacts` | `[]` (all) | Phone numbers that get replies |
| `max_history_per_contact` | `10` | Conversation memory (message pairs) |
| `engine_port` | `8765` | Internal port between bridge and engine |
| `anthropic_api_key` | `""` | Your Anthropic key |

---

## Keep it Running 24/7 (Linux)

```bash
# Install pm2
npm install -g pm2

# Start both processes
pm2 start cpp-engine/build/whatsapp_engine --name cpp-engine
pm2 start node-bridge/bridge.js --name wa-bridge

pm2 save          # Auto-restart on reboot
pm2 logs          # View live logs
```

---

## Troubleshooting

| Problem | Solution |
|---|---|
| `cmake: not found` | `sudo apt install cmake` |
| `libcurl not found` | `sudo apt install libcurl4-openssl-dev` |
| `json.hpp not found` | Run the curl download command in Step 1 |
| Bot not replying | Check sender ID in engine logs, add to `allowed_contacts` |
| Engine timeout | Verify engine is running: `curl -X POST http://127.0.0.1:8765/message -d '{"sender":"test","message":"hi"}'` |

---

## ⚠️ Warnings

1. **WhatsApp ToS**: Using unofficial clients may get your number banned. Test with a secondary number first.
2. **Security**: Don't share `config.json` — it contains your API key.
3. **API Costs**: Monitor usage at https://console.anthropic.com.
