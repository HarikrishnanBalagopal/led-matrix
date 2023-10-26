const W = 13;
const H = W;
const TOTAL_CELL = W * H;

const fract = (x) => x - Math.floor(x);

const pos = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
const vel = [1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1];
// const vel = [1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, 1];

const draw = (ctx, _t, _delta_t) => {
    // const t = _t * 0.001;
    // const t = _t * 0.0001;
    const t = _t * 0.0001;
    const delta_t = _delta_t * 0.01;
    ctx.save();
    // for (let r = 0; r < H; r++) {
    //     for (let c = 0; c < W; c++) {
    //         const idx = r * W + c;
    //         const deg = 360 * fract(t + idx / TOTAL_CELL);
    //         ctx.fillStyle = `hsl(${deg}deg, 100%, 50%)`;
    //         ctx.fillRect(c, r, 1, 1);
    //     }
    // }
    ctx.fillStyle = 'black';
    ctx.fillRect(0, 0, W, H);
    for (let col = 0; col < 13; col++) {
        pos[col] = pos[col] + vel[col] * delta_t;
        if (pos[col] < 0) {
            pos[col] = 0;
            vel[col] *= -1;
        } else if (pos[col] >= 13) {
            pos[col] = 12;
            vel[col] *= -1;
        }
        // ctx.fillStyle = 'green';
        ctx.fillStyle = `hsl(${360 * col / 13}deg, 100%, 50%)`;
        ctx.fillRect(col, Math.floor(pos[col]), 1, 1);
        // for (let row = 0; row < 13; row += 8) {
        //     const rr = Math.floor(pos[col]);
        //     const rc = (rr + row) % 13;
        //     ctx.fillRect(col, rc, 1, 1);
        // }
    }

    // const tt1 = fract(t / 3);
    // ctx.fillStyle = tt1 > 0.6 ? '#0000ff' : (tt1 > 0.3 ? '#00ff00' : '#ff0000');
    // ctx.font = "13px serif";
    // const text = "Happy Dussehra!!";
    // const text_len = text.length * 6;
    // ctx.fillText(text, 10 + fract(t) * -text_len, 10);
    ctx.restore();
};

const main = () => {
    console.log('main start');
    const ws = new WebSocket(`ws://${window.location.host}/ws`);
    ws.binaryType = "arraybuffer";
    const handler_msg = (e) => {
        console.log('websocket message:', e);
        const is_binary = e.data instanceof ArrayBuffer;
        console.log('is_binary', is_binary);
        if (is_binary) {
            const dec = new TextDecoder('utf-8');
            const s = dec.decode(e.data);
            console.log('decode binary message as utf-8 string:', s);
            ws.send('hello from websocket client');
        }
    };

    const setup = () => {
        console.log("setup start");
        const canvas = document.querySelector("canvas");
        canvas.width = W;
        canvas.height = H;
        const ctx = canvas.getContext("2d", { willReadFrequently: true });
        // ctx.rotate(Math.PI);
        // ctx.translate(0, 0);
        ctx.scale(-1, -1);
        ctx.translate(-W, -H);
        ctx.fillRect(0, 0, W, H);
        let last_t = 0;
        const step = (t) => {
            requestAnimationFrame(step);
            const delta_t = t - last_t;
            if (delta_t < 1) return;
            last_t = t;
            draw(ctx, t, delta_t);
            const imgData = ctx.getImageData(0, 0, W, H);
            // console.log('imgData:', imgData);
            ws.send(imgData.data);
        };
        requestAnimationFrame(step);
        console.log("setup end");
    };

    ws.addEventListener('error', (e) => console.error('websocket error:', e));
    ws.addEventListener('open', (e) => {
        console.log('websocket open:', e);
        setup();
    });
    ws.addEventListener('close', (e) => console.log('websocket close:', e));
    ws.addEventListener('message', handler_msg);


    console.log('main end');
};

main();
