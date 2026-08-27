// C Chess Engine - Web Game Client Logic (Strict Chess Rules Enforcement)

const PIECES = {
    P: '♙', N: '♘', B: '♗', R: '♖', Q: '♕', K: '♔',
    p: '♟', n: '♞', b: '♝', r: '♜', q: '♛', k: '♚'
};

const START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

// Web Audio API Synthesizer
class SoundEffects {
    constructor() {
        this.ctx = null;
    }
    init() {
        if (!this.ctx) {
            this.ctx = new (window.AudioContext || window.webkitAudioContext)();
        }
    }
    playMove() {
        this.init();
        if (!this.ctx) return;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'sine';
        osc.frequency.setValueAtTime(320, this.ctx.currentTime);
        osc.frequency.exponentialRampToValueAtTime(120, this.ctx.currentTime + 0.08);
        gain.gain.setValueAtTime(0.3, this.ctx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.01, this.ctx.currentTime + 0.08);
        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.08);
    }
    playCapture() {
        this.init();
        if (!this.ctx) return;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'triangle';
        osc.frequency.setValueAtTime(600, this.ctx.currentTime);
        osc.frequency.exponentialRampToValueAtTime(80, this.ctx.currentTime + 0.12);
        gain.gain.setValueAtTime(0.4, this.ctx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.01, this.ctx.currentTime + 0.12);
        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.12);
    }
    playCheck() {
        this.init();
        if (!this.ctx) return;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'square';
        osc.frequency.setValueAtTime(880, this.ctx.currentTime);
        osc.frequency.setValueAtTime(1100, this.ctx.currentTime + 0.08);
        gain.gain.setValueAtTime(0.2, this.ctx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.01, this.ctx.currentTime + 0.2);
        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.2);
    }
}

const sfx = new SoundEffects();

// Chess Game Logic Engine
class ChessGame {
    constructor() {
        this.board = new Array(64).fill(null);
        this.side = 'w';
        this.castling = { K: true, Q: true, k: true, q: true };
        this.enpassant = null;
        this.halfmove = 0;
        this.fullmove = 1;
        this.history = [];
        this.moveList = [];
        this.isFlipped = false;
        this.playerColor = 'w';
        this.mode = 'human_vs_engine';
        this.depth = 4;
        this.selectedSquare = null;
        this.legalTargets = [];
        this.lastMove = null;
        this.isSearching = false;

        this.loadFen(START_FEN);
    }

    reset() {
        this.history = [];
        this.moveList = [];
        this.lastMove = null;
        this.selectedSquare = null;
        this.legalTargets = [];
        this.loadFen(START_FEN);
    }

    loadFen(fen) {
        this.board.fill(null);
        const parts = fen.trim().split(' ');
        const ranks = parts[0].split('/');

        for (let r = 0; r < 8; r++) {
            let f = 0;
            const rankStr = ranks[r];
            for (let i = 0; i < rankStr.length; i++) {
                const c = rankStr[i];
                if (c >= '1' && c <= '8') {
                    f += parseInt(c, 10);
                } else {
                    const sq = (7 - r) * 8 + f;
                    this.board[sq] = c;
                    f++;
                }
            }
        }

        this.side = parts[1] || 'w';

        const castlingStr = parts[2] || '-';
        this.castling = {
            K: castlingStr.includes('K'),
            Q: castlingStr.includes('Q'),
            k: castlingStr.includes('k'),
            q: castlingStr.includes('q')
        };

        if (parts[3] && parts[3] !== '-') {
            const file = parts[3].charCodeAt(0) - 97;
            const rank = parseInt(parts[3][1], 10) - 1;
            this.enpassant = rank * 8 + file;
        } else {
            this.enpassant = null;
        }

        this.halfmove = parseInt(parts[4] || '0', 10);
        this.fullmove = parseInt(parts[5] || '1', 10);
    }

    generateFen() {
        let fen = '';
        for (let r = 7; r >= 0; r--) {
            let empty = 0;
            for (let f = 0; f < 8; f++) {
                const sq = r * 8 + f;
                const p = this.board[sq];
                if (!p) {
                    empty++;
                } else {
                    if (empty > 0) {
                        fen += empty;
                        empty = 0;
                    }
                    fen += p;
                }
            }
            if (empty > 0) fen += empty;
            if (r > 0) fen += '/';
        }

        fen += ' ' + this.side + ' ';

        let cStr = '';
        if (this.castling.K) cStr += 'K';
        if (this.castling.Q) cStr += 'Q';
        if (this.castling.k) cStr += 'k';
        if (this.castling.q) cStr += 'q';
        fen += (cStr || '-') + ' ';

        if (this.enpassant !== null) {
            const f = String.fromCharCode(97 + (this.enpassant % 8));
            const r = Math.floor(this.enpassant / 8) + 1;
            fen += f + r;
        } else {
            fen += '-';
        }

        fen += ` ${this.halfmove} ${this.fullmove}`;
        return fen;
    }

    getPieceColor(piece) {
        if (!piece) return null;
        return piece === piece.toUpperCase() ? 'w' : 'b';
    }

    squareToAlgebraic(sq) {
        const f = String.fromCharCode(97 + (sq % 8));
        const r = Math.floor(sq / 8) + 1;
        return f + r;
    }

    algebraicToSquare(alg) {
        if (!alg || alg.length < 2) return null;
        const f = alg.charCodeAt(0) - 97;
        const r = parseInt(alg[1], 10) - 1;
        return r * 8 + f;
    }

    // ==========================================
    // STRICT CHESS MOVE GENERATION & LEGALITY
    // ==========================================
    getLegalMovesForSquare(sq) {
        const piece = this.board[sq];
        if (!piece || this.getPieceColor(piece) !== this.side) return [];

        const pseudoMoves = this.getPseudoLegalMoves(sq);
        const legalMoves = [];

        for (const targetSq of pseudoMoves) {
            if (this.isMoveLegal(sq, targetSq)) {
                legalMoves.push(targetSq);
            }
        }

        return legalMoves;
    }

    getPseudoLegalMoves(sq) {
        const piece = this.board[sq];
        if (!piece) return [];

        const color = this.getPieceColor(piece);
        const type = piece.toLowerCase();
        const r = Math.floor(sq / 8);
        const f = sq % 8;
        const targets = [];

        if (type === 'p') {
            const dir = color === 'w' ? 1 : -1;
            const startRank = color === 'w' ? 1 : 6;

            // Single Push
            const f1 = sq + dir * 8;
            if (f1 >= 0 && f1 < 64 && !this.board[f1]) {
                targets.push(f1);
                // Double Push
                const f2 = sq + dir * 16;
                if (r === startRank && !this.board[f2]) {
                    targets.push(f2);
                }
            }

            // Captures
            const capFiles = [f - 1, f + 1];
            for (const cf of capFiles) {
                if (cf >= 0 && cf <= 7) {
                    const capSq = (r + dir) * 8 + cf;
                    if (capSq >= 0 && capSq < 64) {
                        const targetP = this.board[capSq];
                        if (targetP && this.getPieceColor(targetP) !== color) {
                            targets.push(capSq);
                        } else if (capSq === this.enpassant) {
                            targets.push(capSq);
                        }
                    }
                }
            }
        } else if (type === 'n') {
            const dr = [2, 2, 1, 1, -1, -1, -2, -2];
            const df = [1, -1, 2, -2, 2, -2, 1, -1];
            for (let i = 0; i < 8; i++) {
                const tr = r + dr[i];
                const tf = f + df[i];
                if (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
                    const tSq = tr * 8 + tf;
                    const targetP = this.board[tSq];
                    if (!targetP || this.getPieceColor(targetP) !== color) {
                        targets.push(tSq);
                    }
                }
            }
        } else if (type === 'b' || type === 'r' || type === 'q') {
            const directions = [];
            if (type === 'b' || type === 'q') {
                directions.push([1, 1], [1, -1], [-1, 1], [-1, -1]);
            }
            if (type === 'r' || type === 'q') {
                directions.push([1, 0], [-1, 0], [0, 1], [0, -1]);
            }

            for (const [dr, df] of directions) {
                let tr = r + dr;
                let tf = f + df;
                while (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
                    const tSq = tr * 8 + tf;
                    const targetP = this.board[tSq];
                    if (!targetP) {
                        targets.push(tSq);
                    } else {
                        if (this.getPieceColor(targetP) !== color) {
                            targets.push(tSq);
                        }
                        break;
                    }
                    tr += dr;
                    tf += df;
                }
            }
        } else if (type === 'k') {
            const dr = [1, 1, 1, 0, 0, -1, -1, -1];
            const df = [-1, 0, 1, -1, 1, -1, 0, 1];
            for (let i = 0; i < 8; i++) {
                const tr = r + dr[i];
                const tf = f + df[i];
                if (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
                    const tSq = tr * 8 + tf;
                    const targetP = this.board[tSq];
                    if (!targetP || this.getPieceColor(targetP) !== color) {
                        targets.push(tSq);
                    }
                }
            }

            // Castling
            const enemyColor = color === 'w' ? 'b' : 'w';
            if (color === 'w') {
                if (this.castling.K && !this.board[5] && !this.board[6]) {
                    if (!this.isSquareAttacked(4, enemyColor) && !this.isSquareAttacked(5, enemyColor) && !this.isSquareAttacked(6, enemyColor)) {
                        targets.push(6);
                    }
                }
                if (this.castling.Q && !this.board[1] && !this.board[2] && !this.board[3]) {
                    if (!this.isSquareAttacked(4, enemyColor) && !this.isSquareAttacked(3, enemyColor) && !this.isSquareAttacked(2, enemyColor)) {
                        targets.push(2);
                    }
                }
            } else {
                if (this.castling.k && !this.board[61] && !this.board[62]) {
                    if (!this.isSquareAttacked(60, enemyColor) && !this.isSquareAttacked(61, enemyColor) && !this.isSquareAttacked(62, enemyColor)) {
                        targets.push(62);
                    }
                }
                if (this.castling.q && !this.board[57] && !this.board[58] && !this.board[59]) {
                    if (!this.isSquareAttacked(60, enemyColor) && !this.isSquareAttacked(59, enemyColor) && !this.isSquareAttacked(58, enemyColor)) {
                        targets.push(58);
                    }
                }
            }
        }

        return targets;
    }

    isSquareAttacked(sq, attackerColor, tempBoard = this.board) {
        const r = Math.floor(sq / 8);
        const f = sq % 8;

        // Pawns
        const pawnDir = attackerColor === 'w' ? -1 : 1;
        const enemyPawn = attackerColor === 'w' ? 'P' : 'p';
        for (const cf of [f - 1, f + 1]) {
            if (cf >= 0 && cf <= 7) {
                const pSq = (r + pawnDir) * 8 + cf;
                if (pSq >= 0 && pSq < 64 && tempBoard[pSq] === enemyPawn) return true;
            }
        }

        // Knights
        const enemyKnight = attackerColor === 'w' ? 'N' : 'n';
        const knightDr = [2, 2, 1, 1, -1, -1, -2, -2];
        const knightDf = [1, -1, 2, -2, 2, -2, 1, -1];
        for (let i = 0; i < 8; i++) {
            const tr = r + knightDr[i];
            const tf = f + knightDf[i];
            if (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
                if (tempBoard[tr * 8 + tf] === enemyKnight) return true;
            }
        }

        // King
        const enemyKing = attackerColor === 'w' ? 'K' : 'k';
        const kingDr = [1, 1, 1, 0, 0, -1, -1, -1];
        const kingDf = [-1, 0, 1, -1, 1, -1, 0, 1];
        for (let i = 0; i < 8; i++) {
            const tr = r + kingDr[i];
            const tf = f + kingDf[i];
            if (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
                if (tempBoard[tr * 8 + tf] === enemyKing) return true;
            }
        }

        // Sliders (Bishops, Rooks, Queens)
        const enemyBishop = attackerColor === 'w' ? 'B' : 'b';
        const enemyRook = attackerColor === 'w' ? 'R' : 'r';
        const enemyQueen = attackerColor === 'w' ? 'Q' : 'q';

        // Diagonals
        for (const [dr, df] of [[1, 1], [1, -1], [-1, 1], [-1, -1]]) {
            let tr = r + dr;
            let tf = f + df;
            while (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
                const p = tempBoard[tr * 8 + tf];
                if (p) {
                    if (p === enemyBishop || p === enemyQueen) return true;
                    break;
                }
                tr += dr; tf += df;
            }
        }

        // Orthogonals
        for (const [dr, df] of [[1, 0], [-1, 0], [0, 1], [0, -1]]) {
            let tr = r + dr;
            let tf = f + df;
            while (tr >= 0 && tr <= 7 && tf >= 0 && tf <= 7) {
                const p = tempBoard[tr * 8 + tf];
                if (p) {
                    if (p === enemyRook || p === enemyQueen) return true;
                    break;
                }
                tr += dr; tf += df;
            }
        }

        return false;
    }

    isMoveLegal(from, to) {
        const tempBoard = [...this.board];
        const piece = tempBoard[from];
        tempBoard[from] = null;
        tempBoard[to] = piece;

        // Handle En Passant in temp board
        if (piece.toLowerCase() === 'p' && to === this.enpassant) {
            const epCapSq = this.side === 'w' ? to - 8 : to + 8;
            tempBoard[epCapSq] = null;
        }

        // Find King
        const kingChar = this.side === 'w' ? 'K' : 'k';
        let kingSq = -1;
        for (let i = 0; i < 64; i++) {
            if (tempBoard[i] === kingChar) {
                kingSq = i;
                break;
            }
        }

        if (kingSq === -1) return false;

        const enemyColor = this.side === 'w' ? 'b' : 'w';
        return !this.isSquareAttacked(kingSq, enemyColor, tempBoard);
    }

    makeUciMove(uciMove) {
        if (!uciMove || uciMove.length < 4) return false;
        const from = this.algebraicToSquare(uciMove.substring(0, 2));
        const to = this.algebraicToSquare(uciMove.substring(2, 4));
        const promoChar = uciMove.length >= 5 ? uciMove[4] : null;

        return this.executeMove(from, to, promoChar);
    }

    executeMove(from, to, promoChar = null) {
        const piece = this.board[from];
        if (!piece) return false;

        const isCapture = this.board[to] !== null;

        // Save history state
        this.history.push({
            fen: this.generateFen(),
            lastMove: { from, to }
        });

        // Handle Pawn Promotion
        let placedPiece = piece;
        if (piece.toLowerCase() === 'p') {
            const rank = Math.floor(to / 8);
            if (rank === 7 || rank === 0) {
                const pChar = promoChar || 'q';
                placedPiece = this.side === 'w' ? pChar.toUpperCase() : pChar.toLowerCase();
            }
        }

        // Handle En Passant
        if (piece.toLowerCase() === 'p' && to === this.enpassant) {
            const epCapSq = this.side === 'w' ? to - 8 : to + 8;
            this.board[epCapSq] = null;
        }

        // Handle Castling Rook Move
        if (piece.toLowerCase() === 'k' && Math.abs(from - to) === 2) {
            if (to === 6) { // G1
                this.board[7] = null; this.board[5] = 'R';
            } else if (to === 2) { // C1
                this.board[0] = null; this.board[3] = 'R';
            } else if (to === 62) { // G8
                this.board[63] = null; this.board[61] = 'r';
            } else if (to === 58) { // C8
                this.board[56] = null; this.board[59] = 'r';
            }
        }

        // Move piece
        this.board[from] = null;
        this.board[to] = placedPiece;

        // Update En Passant Target
        if (piece.toLowerCase() === 'p' && Math.abs(from - to) === 16) {
            this.enpassant = this.side === 'w' ? from + 8 : from - 8;
        } else {
            this.enpassant = null;
        }

        // Update Castling Rights
        if (from === 0 || to === 0) this.castling.Q = false;
        if (from === 7 || to === 7) this.castling.K = false;
        if (from === 4) { this.castling.K = false; this.castling.Q = false; }
        if (from === 56 || to === 56) this.castling.q = false;
        if (from === 63 || to === 63) this.castling.k = false;
        if (from === 60) { this.castling.k = false; this.castling.q = false; }

        // Switch side
        this.side = this.side === 'w' ? 'b' : 'w';
        if (this.side === 'w') this.fullmove++;

        this.lastMove = { from, to };
        this.moveList.push(this.squareToAlgebraic(from) + this.squareToAlgebraic(to) + (promoChar || ''));

        // Play SFX
        if (isCapture) {
            sfx.playCapture();
        } else {
            sfx.playMove();
        }

        // Check for Game Over (Checkmate / Stalemate / 50-move rule)
        this.checkGameOver();

        return true;
    }

    findKing(color) {
        const k = color === 'w' ? 'K' : 'k';
        for (let i = 0; i < 64; i++) {
            if (this.board[i] === k) return i;
        }
        return -1;
    }

    getRepetitionCount() {
        const currentFenKey = this.generateFen().split(' ').slice(0, 4).join(' ');
        let count = 1;
        for (let i = 0; i < this.history.length; i++) {
            const hFenKey = this.history[i].fen.split(' ').slice(0, 4).join(' ');
            if (hFenKey === currentFenKey) {
                count++;
            }
        }
        return count;
    }

    checkGameOver() {
        let hasAnyLegalMove = false;
        for (let sq = 0; sq < 64; sq++) {
            const piece = this.board[sq];
            if (piece && this.getPieceColor(piece) === this.side) {
                const moves = this.getLegalMovesForSquare(sq);
                if (moves.length > 0) {
                    hasAnyLegalMove = true;
                    break;
                }
            }
        }

        if (!hasAnyLegalMove) {
            this.isGameOver = true;
            const kingSq = this.findKing(this.side);
            const inChk = kingSq !== -1 ? this.isSquareAttacked(kingSq, this.side === 'w' ? 'b' : 'w') : false;

            const modal = document.getElementById('gameOverModal');
            const titleEl = document.getElementById('gameOverTitle');
            const reasonEl = document.getElementById('gameOverReason');

            if (inChk) {
                const winner = this.side === 'w' ? 'Black' : 'White';
                if (titleEl) titleEl.textContent = `Checkmate! 🏆`;
                if (reasonEl) reasonEl.textContent = `${winner} wins by Checkmate!`;
            } else {
                if (titleEl) titleEl.textContent = `Draw! 🤝`;
                if (reasonEl) reasonEl.textContent = `Stalemate - No legal moves available.`;
            }

            if (modal) modal.classList.add('active');
            sfx.playGameOver();
            return true;
        }

        if (this.getRepetitionCount() >= 3) {
            this.isGameOver = true;
            const modal = document.getElementById('gameOverModal');
            const titleEl = document.getElementById('gameOverTitle');
            const reasonEl = document.getElementById('gameOverReason');

            if (titleEl) titleEl.textContent = `Draw! 🤝`;
            if (reasonEl) reasonEl.textContent = `Draw by Threefold Repetition.`;

            if (modal) modal.classList.add('active');
            sfx.playGameOver();
            return true;
        }

        if (this.fifty_move >= 100) {
            this.isGameOver = true;
            const modal = document.getElementById('gameOverModal');
            const titleEl = document.getElementById('gameOverTitle');
            const reasonEl = document.getElementById('gameOverReason');

            if (titleEl) titleEl.textContent = `Draw! 🤝`;
            if (reasonEl) reasonEl.textContent = `50-move rule exceeded.`;

            if (modal) modal.classList.add('active');
            sfx.playGameOver();
            return true;
        }

        return false;
    }

    undo() {
        if (this.history.length === 0) return false;
        const lastState = this.history.pop();
        this.moveList.pop();
        this.loadFen(lastState.fen);
        this.lastMove = lastState.lastMove;
        this.selectedSquare = null;
        this.legalTargets = [];
        this.isGameOver = false;
        return true;
    }
}

// Global App UI Manager
const game = new ChessGame();

document.addEventListener('DOMContentLoaded', () => {
    initUI();
    renderBoard();
    updateUI();
});

function initUI() {
    document.getElementById('btnPlayWhite').addEventListener('click', () => setPlayerColor('w'));
    document.getElementById('btnPlayBlack').addEventListener('click', () => setPlayerColor('b'));
    document.getElementById('modeSelect').addEventListener('change', (e) => {
        game.mode = e.target.value;
        checkEngineTurn();
    });
    document.getElementById('difficultySelect').addEventListener('change', (e) => {
        game.depth = parseInt(e.target.value, 10);
    });

    // Theme selector
    const savedTheme = localStorage.getItem('chess_theme') || 'slate';
    document.body.setAttribute('data-theme', savedTheme);
    const themeSelect = document.getElementById('themeSelect');
    if (themeSelect) {
        themeSelect.value = savedTheme;
        themeSelect.addEventListener('change', (e) => {
            const theme = e.target.value;
            document.body.setAttribute('data-theme', theme);
            localStorage.setItem('chess_theme', theme);
        });
    }

    document.getElementById('btnNewGame').addEventListener('click', () => {
        game.reset();
        renderBoard();
        updateUI();
        checkEngineTurn();
    });

    document.getElementById('btnUndo').addEventListener('click', () => {
        if (game.mode === 'human_vs_engine') {
            game.undo(); // Undo engine move
            game.undo(); // Undo human move
        } else {
            game.undo();
        }
        renderBoard();
        updateUI();
    });

    document.getElementById('btnHint').addEventListener('click', requestHint);

    document.getElementById('btnFlip').addEventListener('click', () => {
        game.isFlipped = !game.isFlipped;
        renderBoard();
    });

    document.getElementById('btnCopyFen').addEventListener('click', () => {
        const fen = game.generateFen();
        navigator.clipboard.writeText(fen);
        alert('FEN copied to clipboard!');
    });

    document.getElementById('btnModalPlayAgain').addEventListener('click', () => {
        document.getElementById('gameOverModal').classList.remove('active');
        game.reset();
        renderBoard();
        updateUI();
    });
}

function setPlayerColor(color) {
    game.playerColor = color;
    document.getElementById('btnPlayWhite').classList.toggle('active', color === 'w');
    document.getElementById('btnPlayBlack').classList.toggle('active', color === 'b');
    game.isFlipped = (color === 'b');
    renderBoard();
    checkEngineTurn();
}

function renderBoard() {
    const boardEl = document.getElementById('chessboard');
    boardEl.innerHTML = '';

    for (let row = 0; row < 8; row++) {
        for (let col = 0; col < 8; col++) {
            const r = game.isFlipped ? row : (7 - row);
            const f = game.isFlipped ? (7 - col) : col;
            const sq = r * 8 + f;

            const sqEl = document.createElement('div');
            const isLight = (r + f) % 2 === 1;
            sqEl.className = `square ${isLight ? 'light' : 'dark'}`;
            sqEl.dataset.square = sq;

            if (game.selectedSquare === sq) {
                sqEl.classList.add('selected');
            }

            if (game.lastMove && (game.lastMove.from === sq || game.lastMove.to === sq)) {
                sqEl.classList.add('last-move');
            }

            if (game.legalTargets.includes(sq)) {
                sqEl.classList.add('legal-target');
                if (game.board[sq]) sqEl.classList.add('has-piece');
            }

            // Piece Render
            const piece = game.board[sq];
            if (piece) {
                const pieceEl = document.createElement('span');
                const pColor = game.getPieceColor(piece);
                pieceEl.className = `piece ${pColor === 'w' ? 'piece-white' : 'piece-black'}`;
                pieceEl.textContent = PIECES[piece] || '';
                sqEl.appendChild(pieceEl);
            }

            // Rank & File Coordinates
            if (col === 0) {
                const rankEl = document.createElement('span');
                rankEl.className = 'coord-rank';
                rankEl.textContent = r + 1;
                sqEl.appendChild(rankEl);
            }
            if (row === 7) {
                const fileEl = document.createElement('span');
                fileEl.className = 'coord-file';
                fileEl.textContent = String.fromCharCode(97 + f);
                sqEl.appendChild(fileEl);
            }

            sqEl.addEventListener('click', () => handleSquareClick(sq));
            boardEl.appendChild(sqEl);
        }
    }
}

function handleSquareClick(sq) {
    if (game.isSearching || game.isGameOver) return;

    // Check turn constraints
    if (game.mode === 'human_vs_engine' && game.side !== game.playerColor) {
        return;
    }

    const piece = game.board[sq];
    const pieceColor = game.getPieceColor(piece);

    if (game.selectedSquare === null) {
        if (piece && pieceColor === game.side) {
            game.selectedSquare = sq;
            // Generate STRICT LEGAL MOVES for piece
            game.legalTargets = game.getLegalMovesForSquare(sq);
            renderBoard();
        }
    } else {
        if (game.selectedSquare === sq) {
            game.selectedSquare = null;
            game.legalTargets = [];
            renderBoard();
            return;
        }

        if (piece && pieceColor === game.side) {
            game.selectedSquare = sq;
            game.legalTargets = game.getLegalMovesForSquare(sq);
            renderBoard();
            return;
        }

        // Verify that target square is strictly legal
        if (!game.legalTargets.includes(sq)) {
            return;
        }

        // Execute Move
        const from = game.selectedSquare;
        const to = sq;

        // Pawn promotion check
        const movingPiece = game.board[from];
        if (movingPiece && movingPiece.toLowerCase() === 'p') {
            const targetRank = Math.floor(to / 8);
            if (targetRank === 7 || targetRank === 0) {
                showPromotionModal((choice) => {
                    executeUserMove(from, to, choice);
                });
                return;
            }
        }

        executeUserMove(from, to, null);
    }
}

function executeUserMove(from, to, promoChar) {
    const success = game.executeMove(from, to, promoChar);
    game.selectedSquare = null;
    game.legalTargets = [];
    renderBoard();
    updateUI();

    if (success && !game.isGameOver) {
        setTimeout(checkEngineTurn, 100);
    }
}

function showPromotionModal(callback) {
    const modal = document.getElementById('promotionModal');
    const container = document.getElementById('promotionChoices');
    container.innerHTML = '';

    const choices = game.side === 'w' ? ['Q', 'R', 'B', 'N'] : ['q', 'r', 'b', 'n'];
    choices.forEach(c => {
        const el = document.createElement('span');
        el.className = 'promotion-piece';
        el.textContent = PIECES[c];
        el.addEventListener('click', () => {
            modal.classList.remove('active');
            callback(c.toLowerCase());
        });
        container.appendChild(el);
    });

    modal.classList.add('active');
}

function checkEngineTurn() {
    if (game.isGameOver || game.mode === 'human_vs_human') return;

    const isEngineTurn = (game.mode === 'engine_vs_engine') || (game.mode === 'human_vs_engine' && game.side !== game.playerColor);

    if (isEngineTurn) {
        triggerEngineMove();
    }
}

function triggerEngineMove() {
    if (game.isGameOver) return;
    if (game.isSearching) {
        // Retry shortly if an active search is already running
        setTimeout(checkEngineTurn, 300);
        return;
    }

    game.isSearching = true;
    const fen = game.generateFen();

    const searchTimer = setTimeout(() => {
        console.warn('Engine search safety timer triggered.');
        game.isSearching = false;
    }, 12000);

    fetch('/api/bestmove', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ fen, depth: game.depth })
    })
    .then(res => res.json())
    .then(data => {
        clearTimeout(searchTimer);
        game.isSearching = false;

        if (data.bestmove && data.bestmove !== '0000') {
            game.makeUciMove(data.bestmove);
            renderBoard();
            updateUI(data.score);

            if (!game.isGameOver && game.mode === 'engine_vs_engine') {
                setTimeout(checkEngineTurn, 600);
            }
        } else {
            // Engine returned 0000 -> Check if position is actually Game Over
            const isOver = game.checkGameOver();
            renderBoard();
            updateUI();

            if (!isOver && (game.mode === 'engine_vs_engine' || (game.mode === 'human_vs_engine' && game.side !== game.playerColor))) {
                console.warn('Engine returned 0000 but position is not game over. Retrying engine move...');
                setTimeout(checkEngineTurn, 500);
            }
        }
    })
    .catch(err => {
        console.error('Engine API Error:', err);
        clearTimeout(searchTimer);
        game.isSearching = false;
    });
}

function requestHint() {
    const fen = game.generateFen();
    fetch('/api/bestmove', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ fen, depth: Math.max(game.depth, 6) })
    })
    .then(res => res.json())
    .then(data => {
        if (data.bestmove) {
            const fromAlg = data.bestmove.substring(0, 2);
            const toAlg = data.bestmove.substring(2, 4);
            const fromSq = game.algebraicToSquare(fromAlg);
            const toSq = game.algebraicToSquare(toAlg);

            game.selectedSquare = fromSq;
            game.legalTargets = [toSq];
            renderBoard();
        }
    });
}

function updateUI(engineScore = 0) {
    // Update FEN
    document.getElementById('fenInput').value = game.generateFen();

    // Update Timers & Names
    const isWhiteTurn = game.side === 'w';
    document.getElementById('bottomTimer').classList.toggle('active', isWhiteTurn);
    document.getElementById('topTimer').classList.toggle('active', !isWhiteTurn);

    const topNameEl = document.getElementById('topPlayerName');
    const bottomNameEl = document.getElementById('bottomPlayerName');

    if (game.mode === 'human_vs_human') {
        topNameEl.textContent = 'Player 2 (Black)';
        bottomNameEl.textContent = 'Player 1 (White)';
    } else if (game.mode === 'engine_vs_engine') {
        topNameEl.textContent = 'C Engine 2 (Black)';
        bottomNameEl.textContent = 'C Engine 1 (White)';
    } else {
        if (game.playerColor === 'w') {
            topNameEl.textContent = 'C Engine (Black)';
            bottomNameEl.textContent = 'Player (White)';
        } else {
            topNameEl.textContent = 'Player (White)';
            bottomNameEl.textContent = 'C Engine (Black)';
        }
    }

    // Update Eval Bar
    let scoreDisplay = '+0.0';
    let evalPct = 50;

    if (engineScore !== undefined) {
        const cp = game.side === 'w' ? engineScore : -engineScore;
        const evalVal = (cp / 100).toFixed(1);
        scoreDisplay = (cp >= 0 ? '+' : '') + evalVal;
        evalPct = Math.min(Math.max(50 + (cp / 20), 5), 95);
    }

    document.getElementById('evalFill').style.height = `${evalPct}%`;
    document.getElementById('evalBadge').textContent = scoreDisplay;

    // Update History Table
    renderHistoryTable();
}

function renderHistoryTable() {
    const tbody = document.getElementById('historyTableBody');
    tbody.innerHTML = '';

    for (let i = 0; i < game.moveList.length; i += 2) {
        const moveNum = Math.floor(i / 2) + 1;
        const whiteMove = game.moveList[i] || '';
        const blackMove = game.moveList[i + 1] || '';

        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${moveNum}.</td>
            <td>${whiteMove}</td>
            <td>${blackMove}</td>
            <td>-</td>
        `;
        tbody.appendChild(tr);
    }

    const container = document.getElementById('historyContainer');
    container.scrollTop = container.scrollHeight;
}
