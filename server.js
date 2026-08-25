const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const PORT = 3000;
const ENGINE_PATH = process.platform === 'win32' ? './chess_engine.exe' : './chess_engine';

let engineProcess = null;
let currentCallback = null;
let engineOutputBuffer = '';

// Start native C chess engine process in UCI mode
function startEngine() {
    console.log(`[Server] Spawning C Chess Engine process: ${ENGINE_PATH}`);
    engineProcess = spawn(ENGINE_PATH, ['uci']);

    engineProcess.stdout.on('data', (data) => {
        const text = data.toString();
        engineOutputBuffer += text;

        if (currentCallback) {
            currentCallback(text);
        }
    });

    engineProcess.stderr.on('data', (data) => {
        console.error(`[Engine Error] ${data.toString()}`);
    });

    engineProcess.on('close', (code) => {
        console.log(`[Engine] Process exited with code ${code}`);
        engineProcess = null;
    });

    // Initialize UCI protocol
    sendEngineCommand('uci');
    sendEngineCommand('isready');
}

function sendEngineCommand(cmd) {
    if (engineProcess && engineProcess.stdin.writable) {
        engineProcess.stdin.write(cmd + '\n');
    }
}

let isEngineSearching = false;
let searchTimeout = null;

// Helper to query engine with position and search depth
function queryEngineMove(fen, depth, callback) {
    if (!engineProcess) startEngine();

    if (isEngineSearching && searchTimeout) {
        clearTimeout(searchTimeout);
    }
    isEngineSearching = true;

    let lastScore = 0;
    let lastPv = [];
    let bestMove = null;

    const cleanup = () => {
        if (searchTimeout) clearTimeout(searchTimeout);
        searchTimeout = null;
        currentCallback = null;
        engineOutputBuffer = '';
        isEngineSearching = false;
    };

    const onData = (data) => {
        const lines = engineOutputBuffer.split('\n');
        for (let line of lines) {
            line = line.trim();
            if (line.startsWith('info depth')) {
                const cpMatch = line.match(/score cp (-?\d+)/);
                if (cpMatch) lastScore = parseInt(cpMatch[1], 10);

                const mateMatch = line.match(/score mate (-?\d+)/);
                if (mateMatch) {
                    const m = parseInt(mateMatch[1], 10);
                    lastScore = m > 0 ? 10000 - m : -10000 - m;
                }

                const pvIndex = line.indexOf(' pv ');
                if (pvIndex !== -1) {
                    lastPv = line.substring(pvIndex + 4).trim().split(' ');
                }
            } else if (line.startsWith('bestmove')) {
                const parts = line.split(' ');
                bestMove = parts[1];

                cleanup();
                callback({
                    bestmove: bestMove,
                    score: lastScore,
                    depth: depth,
                    pv: lastPv
                });
                return;
            }
        }
    };

    searchTimeout = setTimeout(() => {
        console.warn('[Server] Engine search timeout, stopping engine...');
        sendEngineCommand('stop');
        cleanup();
        callback({
            bestmove: '0000',
            score: 0,
            depth: depth,
            pv: []
        });
    }, 10000);

    engineOutputBuffer = '';
    currentCallback = onData;

    sendEngineCommand(`position fen ${fen}`);
    sendEngineCommand(`go depth ${depth}`);
}

// HTTP Request Handler
const requestHandler = (req, res) => {
    if (req.method === 'POST' && req.url === '/api/bestmove') {
        let body = '';
        req.on('data', chunk => { body += chunk.toString(); });
        req.on('end', () => {
            try {
                const data = JSON.parse(body);
                const fen = data.fen || 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';
                const depth = data.depth || 6;

                queryEngineMove(fen, depth, (result) => {
                    res.writeHead(200, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify(result));
                });
            } catch (err) {
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: 'Invalid JSON request' }));
            }
        });
        return;
    }

    if (req.method === 'GET' && req.url === '/api/status') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ status: 'ok', engine: 'C Chess Engine 1.0' }));
        return;
    }

    // Serve static files from web/ directory
    let filePath = path.join(__dirname, 'web', req.url === '/' ? 'index.html' : req.url);
    const ext = path.extname(filePath);

    const mimeTypes = {
        '.html': 'text/html',
        '.css': 'text/css',
        '.js': 'text/javascript',
        '.json': 'application/json',
        '.png': 'image/png',
        '.svg': 'image/svg+xml'
    };

    const contentType = mimeTypes[ext] || 'text/plain';

    fs.readFile(filePath, (err, content) => {
        if (err) {
            if (err.code === 'ENOENT') {
                res.writeHead(404, { 'Content-Type': 'text/html' });
                res.end('<h1>404 Not Found</h1>', 'utf-8');
            } else {
                res.writeHead(500);
                res.end(`Server Error: ${err.code}`);
            }
        } else {
            res.writeHead(200, { 'Content-Type': contentType });
            res.end(content, 'utf-8');
        }
    });
};

// Start Server & Engine
const server = http.createServer(requestHandler);

server.on('error', (err) => {
    if (err.code === 'EADDRINUSE') {
        console.log(`[Server] Port ${PORT} is in use, trying port 3001...`);
        server.listen(3001);
    } else {
        console.error('[Server Error]', err);
    }
});

server.listen(PORT, () => {
    console.log(`\n =======================================================`);
    console.log(`   C CHESS ENGINE WEB GAME RUNNING!`);
    console.log(`   Open in browser: http://localhost:${server.address().port}`);
    console.log(` =======================================================\n`);
    startEngine();
});
