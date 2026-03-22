// ============================================================
//  WhatsApp Bridge (Node.js)
//  Connects WhatsApp → forwards each message to C++ engine
//  → sends the reply back.
// ============================================================

const { Client, LocalAuth } = require('whatsapp-web.js');
const qrcode = require('qrcode-terminal');
const http   = require('http');

const ENGINE_HOST = '127.0.0.1';
const ENGINE_PORT = 8765;
const REPLY_DELAY_MS = 1500;  // typing delay before sending

// ── Forward a message to the C++ engine ─────────────────────
function askEngine(sender, message) {
    return new Promise((resolve, reject) => {
        const body = JSON.stringify({ sender, message });
        const req  = http.request(
            { host: ENGINE_HOST, port: ENGINE_PORT, method: 'POST',
              path: '/message',
              headers: { 'Content-Type': 'application/json',
                         'Content-Length': Buffer.byteLength(body) } },
            (res) => {
                let data = '';
                res.on('data', d => data += d);
                res.on('end', () => {
                    try { resolve(JSON.parse(data).reply); }
                    catch { reject(new Error('Bad response from engine')); }
                });
            }
        );
        req.on('error', reject);
        req.setTimeout(30000, () => { req.destroy(); reject(new Error('Engine timeout')); });
        req.write(body);
        req.end();
    });
}

// ── WhatsApp client ──────────────────────────────────────────
const client = new Client({
    authStrategy: new LocalAuth({ clientId: 'wa-bridge' }),
    puppeteer: { headless: true, args: ['--no-sandbox', '--disable-setuid-sandbox'] }
});

client.on('qr', (qr) => {
    console.log('\n📱 Scan this QR code with WhatsApp:\n');
    qrcode.generate(qr, { small: true });
    console.log('\nOpen WhatsApp → Linked Devices → Link a Device\n');
});

client.on('authenticated', () => console.log('✅ WhatsApp authenticated!'));

client.on('ready', () => {
    console.log('🟢 Bridge ready — forwarding messages to C++ engine on port', ENGINE_PORT, '\n');
});

client.on('message', async (msg) => {
    if (msg.fromMe || msg.isGroupMsg) return;

    const sender  = msg.from;
    const message = msg.body;

    try {
        const reply = await askEngine(sender, message);

        if (reply === '__SKIP__' || reply === '__ERROR__') {
            console.log(`[Bridge] Engine skipped message from ${sender}`);
            return;
        }

        // Show typing indicator then send
        const chat = await msg.getChat();
        await chat.sendStateTyping();
        await new Promise(r => setTimeout(r, REPLY_DELAY_MS));
        await msg.reply(reply);
        console.log(`[Bridge] Replied to ${sender}`);

    } catch (err) {
        console.error('[Bridge] Error:', err.message);
    }
});

client.on('disconnected', (reason) => {
    console.log('🔴 Disconnected:', reason, '— reconnecting in 5s...');
    setTimeout(() => client.initialize(), 5000);
});

client.initialize();
