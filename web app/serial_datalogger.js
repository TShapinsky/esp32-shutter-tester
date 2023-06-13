
class SerialDatalogger {
    constructor(port, div, trigger_div, arm_button, status_span, trigger_data_div, timebase = 0.1, draw_interval = 10, sample_rate = 200000, channel_count = 4) {
        this.channel_count = channel_count;
        this.sample_rate = sample_rate;
        this.timebase = timebase;
        this.div = div;
        this.trigger_div = trigger_div;
        this.arm_button = arm_button;
        this.status_span = status_span;
        this.trigger_data_div = trigger_data_div;
        this.setup_canvas();

        this.port = port;
        this.draw_interval = draw_interval;

        this.input_buffer = new CircularBuffer(new Uint8Array(sample_rate*channel_count*2));
        this.data_buffer = new CircularBuffer(new Uint8Array(sample_rate*channel_count*2), true);

        this.chunk_size = 1024;
        this.pixel_oversample_ratio = 5;
        this.max_adc = 255;
        this.trigger = this.max_adc/2;



        this.setup_draw_buffer();
        this.draw_handle = null;
        this.data_check_handle = null;
        this.armed = false;

        addEventListener("resize", (event) => {
            this.update_canvas();
            this.setup_draw_buffer();
        })

        this.arm_button.addEventListener('click', () => {
            if (this.armed) {
                this.disarm();
            } else {
                this.arm();
            }
        });
    }

    set_status(status) {
        this.status_span.textContent = status;
    }

    set_timebase(timebase) {
        this.timebase = timebase;
        this.setup_draw_buffer();
    }

    setup_canvas(){
        this.set_status("Initializing Canvases");
        let canvas_str = `<canvas id="base-canvas" style="position: absolute; left: 0; top: 0; z-index: 0; width: 100%; height: 100%"></canvas>`;
        for (let i =0; i < this.channel_count; i++) {
            canvas_str += `<canvas id="canvas-${i}" style="position: absolute; left:0; top: 0; z-index: ${i+1}; width: 100%; height: 100%"></canvas>`
        }
        canvas_str += `<canvas id="text-canvas" style="position: absolute; left: 0; top: 0; z-index: 0; width: 100%; height: 100%"></canvas>`;
        canvas_str += `<canvas id="control-canvas" style="position: absolute; left: 0; top: 0; z-index: 0; width: 100%; height: 100%"></canvas>`;

        this.div.innerHTML = canvas_str;

        this.base_canvas = document.getElementById("base-canvas");
        this.base_canvas_ctx = this.base_canvas.getContext("2d");

        this.control_canvas = document.getElementById("control-canvas");
        this.control_canvas_ctx = this.control_canvas.getContext("2d");

        this.text_canvas = document.getElementById("text-canvas");
        this.text_canvas_ctx = this.text_canvas.getContext("2d");

        this.canvas = [];
        this.canvas_ctx = [];

        for (let i = 0; i < this.channel_count; i++) {
            let canvas = document.getElementById(`canvas-${i}`);
            this.canvas.push(canvas);
            this.canvas_ctx.push(canvas.getContext("2d"));
        }

        this.div.addEventListener('mousedown', (event) => {
            console.log("mouse down");
            if(this.check_trigger_bounding_box(event)) {
                let listener = (event) => {
                    this.move_trigger(event);
                }
                this.div.addEventListener('mousemove', listener);
                this.div.addEventListener('mouseup', (event) => {
                    this.div.removeEventListener('mousemove', listener);
                })
            }
        });


        let trigger_canvas_str = `<canvas id="trigger-base-canvas" style="position: absolute; left: 0; top: 0; z-index: 0; width: 100%; height: 100%"></canvas>`;
        for (let i =0; i < this.channel_count; i++) {
            trigger_canvas_str += `<canvas id="trigger-${i}" style="position: absolute; left:0; top: 0; z-index: ${i+1}; width: 100%; height: 100%"></canvas>`
        }
        // trigger_canvas_str += `<canvas id="trigger-data-canvas" style="position: absolute; left: 0; top: 0; z-index: 0; width: 100%; height: 100%"></canvas>`;
        this.trigger_div.innerHTML = trigger_canvas_str;

        this.trigger_base_canvas = document.getElementById("trigger-base-canvas");
        this.trigger_base_canvas_ctx = this.trigger_base_canvas.getContext("2d");

        this.trigger_canvas = [];
        this.trigger_canvas_ctx = [];
        for (let i = 0; i < this.channel_count; i++) {
            let canvas = document.getElementById(`trigger-${i}`);
            this.trigger_canvas.push(canvas);
            this.trigger_canvas_ctx.push(canvas.getContext("2d"));
        }


        this.update_canvas();
    }
    
    async start() {
        this.draw_handle = setInterval(() => {this.draw()}, this.draw_interval);
        setTimeout(() => {
            this.set_status("Calculating sample rate");
            let start = Date.now();
            this.input_buffer.sample_count = 0;
            setTimeout(() => {
                let ellapsed = Date.now() - start;
                let samples = this.input_buffer.sample_count;
                let sample_rate = Math.floor(samples/ellapsed*1000/this.channel_count);
                if (sample_rate != 0) {
                    this.sample_rate = sample_rate;
                }
                this.setup_draw_buffer();
                this.set_status(`Calculated sample rate: ${sample_rate}`);
                this.arm_button.disabled = false;
            }, 3000)
        }, 2000);
        this.set_status("Starting port reader");
        await this.start_reading();

    }

    arm () {
        this.armed = true;
        this.arm_button.textContent = "Disarm";
        this.data_buffer.clear();
        clearInterval(this.data_check_handle);
        this.data_check_handle = setInterval(() => {this.check_data()}, 500);
        this.draw_controls();
    }

    disarm () {
        clearInterval(this.data_check_handle);
        this.armed = false;
        this.arm_button.textContent = "Arm";
        this.draw_controls();
    }
    
    update_canvas() {
        this.set_status("Configuring Canvases");
        this.base_canvas.width = this.div.offsetWidth;
        this.base_canvas.height = this.div.offsetHeight;
        this.control_canvas.width = this.div.offsetWidth;
        this.control_canvas.height = this.div.offsetHeight;
        this.text_canvas.width = this.div.offsetWidth;
        this.text_canvas.height = this.div.offsetHeight;

        this.trigger_base_canvas.width = this.trigger_div.offsetWidth;
        this.trigger_base_canvas.height = this.trigger_div.offsetHeight;
        let colors = [
            'red',
            'lime',
            'aqua',
            'white'
        ];
        for (let i = 0; i < this.channel_count; i++) {
            this.canvas[i].width = this.div.offsetWidth;
            this.canvas[i].height = this.div.offsetHeight;
            this.canvas_ctx[i].strokeStyle = colors[i];

            this.trigger_canvas[i].width = this.trigger_div.offsetWidth;
            this.trigger_canvas[i].height = this.trigger_div.offsetHeight;
            this.trigger_canvas_ctx[i].strokeStyle = colors[i];
        }

        this.canvas_width = this.canvas[0].width;
        this.canvas_height = this.canvas[0].height;
        console.log(this.canvas_width);
        console.log(this.canvas_height);
        

        this.base_canvas_ctx.strokeStyle = "#ccc";
        this.base_canvas_ctx.lineWidth = 0.5;
        this.base_canvas_ctx.fillRect(0,0,this.canvas_width, this.canvas_height);
        for (let x = 0; x < this.canvas_width; x += 100) {
            this.base_canvas_ctx.beginPath();
            this.base_canvas_ctx.moveTo(x,0);
            this.base_canvas_ctx.lineTo(x,this.canvas_height);
            this.base_canvas_ctx.stroke();
        }
        for (let i = 0; i < this.max_adc; i += 512) {
            this.base_canvas_ctx.beginPath();
            this.base_canvas_ctx.moveTo(0,i/this.max_adc * this.canvas_height);
            this.base_canvas_ctx.lineTo(this.canvas_width,i/this.max_adc * this.canvas_height);
            this.base_canvas_ctx.stroke();
        }

        this.trigger_base_canvas_ctx.fillRect(0,0,this.canvas_width, this.canvas_height);
        this.set_status("Done Configuring Canvases");
    }
    
    setup_draw_buffer() {
        this.set_status("Setting up draw buffer");
        this.samples_per_pixel = Math.ceil(this.sample_rate*this.timebase/100)
        this.draw_buffer = new CircularBuffer(new Uint8Array(this.samples_per_pixel*this.canvas_width*this.channel_count), true);
        console.log(this.draw_buffer.buffer_length);
        console.log(this.samples_per_pixel);
        this.draw_controls();
        this.set_status("Done setting up draw buffer");
    }

    async start_reading() {
        let input_buffer = this.input_buffer;
        return this.port.readable.pipeTo(new WritableStream({
            write(chunk) {
                input_buffer.push(chunk);
            }
        }));
    }

    process_buffers() {
        let chunk_buffer = new Uint8Array(this.chunk_size);
        this.input_buffer.pop(chunk_buffer);
        // chunk_buffer = new Uint16Array(chunk_buffer.buffer);
        this.draw_buffer.push(chunk_buffer);
        if (this.armed) {
            this.data_buffer.push(chunk_buffer);
        }
    }

    draw() {
        while(this.input_buffer.length > this.chunk_size) {
            this.process_buffers();
        }
        for (let i = 0; i < this.channel_count; i++){
            this.canvas_ctx[i].clearRect(0,0,this.canvas_width, this.canvas_height);
            this.canvas_ctx[i].beginPath();
            this.canvas_ctx[i].moveTo(0, this.canvas_height - this.draw_buffer.buffer[i]/this.max_adc*this.canvas_height);
        }
        let x=0;
        for (let i =0; i+(this.channel_count*this.samples_per_pixel - 1) < this.draw_buffer.length; i += this.channel_count*this.samples_per_pixel) {
            let average_values = new Array(this.channel_count);
            average_values.fill(0);
            for (let j = 0; j < this.samples_per_pixel; j += Math.floor(this.samples_per_pixel/this.pixel_oversample_ratio)) {
                for (let channel = 0; channel < this.channel_count; channel++) {
                    average_values[channel] += this.draw_buffer.buffer[i+j*this.channel_count + channel]
                }
            }
            for ( let channel = 0; channel < this.channel_count; channel++) {
                this.canvas_ctx[channel].lineTo(x, this.canvas_height - average_values[channel]/this.pixel_oversample_ratio/this.max_adc*this.canvas_height);
            }
            x++;
        }
        for (let i = 0; i < this.channel_count; i++) {
            this.canvas_ctx[i].stroke();
        }
    }

    check_trigger_bounding_box(mouse_down_event) {
        let trigger_y = this.trigger/this.max_adc * this.canvas_height;
        let y = mouse_down_event.offsetY;
        console.log(trigger_y);
        console.log(y);

        if (Math.abs(trigger_y - y) < 100) {
            return true;
        }
        return false;
    }

    move_trigger(mouse_move_event) {
        console.log("moves");
        this.trigger = (mouse_move_event.offsetY/this.canvas_height)*this.max_adc;
        this.draw_controls();
    }

    draw_controls() {
        this.control_canvas_ctx.clearRect(0,0,this.canvas_width, this.canvas_height);

        this.control_canvas_ctx.strokeStyle = "yellow";
        //Draw trigger
        this.control_canvas_ctx.beginPath();
        this.control_canvas_ctx.moveTo(0, this.trigger/this.max_adc * this.canvas_height);
        this.control_canvas_ctx.lineTo(this.canvas_width, this.trigger/this.max_adc * this.canvas_height);
        this.control_canvas_ctx.stroke();

        this.control_canvas_ctx.fillStyle = "red"
        if (this.armed) {
            this.control_canvas_ctx.beginPath();
            this.control_canvas_ctx.ellipse(this.canvas_width - 50, 50, 25, 25, 0, 0, Math.PI * 2);
            this.control_canvas_ctx.fill();
        }
    }

    draw_data(data, rising_edge, falling_edge) {
        for (let i = 0; i < this.channel_count; i++){
            this.trigger_canvas_ctx[i].clearRect(0,0,this.canvas_width, this.canvas_height);
            this.trigger_canvas_ctx[i].beginPath();
            this.trigger_canvas_ctx[i].moveTo(0, this.canvas_height - data[i]/this.max_adc*this.canvas_height);
        }
        let x=0;

        let samples_per_pixel = Math.max(Math.floor(data.length/this.channel_count/this.canvas_width), 1);

        for (let i =0; i+(this.channel_count*samples_per_pixel - 1) < data.length; i += this.channel_count*samples_per_pixel) {
            let average_values = new Array(this.channel_count);
            average_values.fill(0);
            for (let j = 0; j < samples_per_pixel; j++) {
                for (let channel = 0; channel < this.channel_count; channel++) {
                    average_values[channel] += data[i+j*this.channel_count + channel]
                }
            }
            for ( let channel = 0; channel < this.channel_count; channel++) {
                this.trigger_canvas_ctx[channel].lineTo(x, this.canvas_height*.75 - average_values[channel]/samples_per_pixel/this.max_adc*this.canvas_height/2);
            }
            x++;
        }
        for (let i = 0; i < this.channel_count; i++) {
            this.trigger_canvas_ctx[i].stroke();
            this.trigger_canvas_ctx[i].setLineDash([5,5]);
            this.trigger_canvas_ctx[i].beginPath();
            this.trigger_canvas_ctx[i].moveTo(rising_edge[i]/samples_per_pixel, 0);
            this.trigger_canvas_ctx[i].lineTo(rising_edge[i]/samples_per_pixel, this.canvas_height);
            this.trigger_canvas_ctx[i].stroke();
            this.trigger_canvas_ctx[i].beginPath();
            this.trigger_canvas_ctx[i].moveTo(falling_edge[i]/samples_per_pixel, 0);
            this.trigger_canvas_ctx[i].lineTo(falling_edge[i]/samples_per_pixel, this.canvas_height);
            this.trigger_canvas_ctx[i].stroke();
            this.trigger_canvas_ctx[i].setLineDash([]);
        }
    }

    check_data() {
        console.log(this.data_buffer.length);
        let data = new Uint16Array(this.data_buffer.length);
        this.data_buffer.pop(data, false);

        const hysteresis = 20;

        let init_count = new Array(this.channel_count);
        init_count.fill(0);
        let init_index = 0;

        let rising_edge_count = new Array(this.channel_count);
        rising_edge_count.fill(0);
        let rising_edge = new Array(this.channel_count);
        rising_edge.fill(0);

        let falling_edge_count = new Array(this.channel_count);
        falling_edge_count.fill(0);
        let falling_edge = new Array(this.channel_count);
        falling_edge.fill(data.length);


        for (let i = 0; i < data.length; i ++) {
            let channel = i%this.channel_count;
            if (data[i] < this.trigger) {
                init_count[channel] += 1;
                if (Math.min(...init_count) >= hysteresis) {
                    init_index = Math.floor(i/this.channel_count) - hysteresis;
                    break;
                }
            } else {
                init_count[channel] = 0;
            }

        } 

        
        for (let i = init_index*this.channel_count; i < data.length; i++) {
            let channel = i%this.channel_count;
            if (rising_edge[channel]) {
                continue;
            }
            if (data[i] > this.trigger) {
                rising_edge_count[channel] += 1;
                if (rising_edge_count[channel] > hysteresis) {
                    rising_edge[channel] = Math.floor(i/this.channel_count) - hysteresis;
                }
            } else {
                rising_edge_count[channel] = 0;
            }
        }

        for (let i = Math.min(...rising_edge)*this.channel_count; i < data.length; i++) {
            let channel = i%this.channel_count;
            let channel_index = Math.floor(i/this.channel_count)
            if (falling_edge[channel] != data.length || channel_index < rising_edge[channel]) {
                continue;
            }
            if (data[i] < this.trigger) {
                falling_edge_count[channel] += 1;
                if (falling_edge_count[channel] > hysteresis) {
                    falling_edge[channel] = Math.floor(i/this.channel_count) - hysteresis;
                }
            } else {
                falling_edge_count[channel] = 0;
            }
        }

        if (Math.min(...rising_edge) != 0 && Math.max(...falling_edge) != data.length) {
            let start = Math.min(...rising_edge);
            let end = Math.max(...falling_edge);
            let range = end - start;
            start -= Math.floor(range*2);
            end += Math.floor(range*2);
            start = Math.max(start, 0);
            end = Math.min(end, data.length/this.channel_count);
            rising_edge = rising_edge.map((i) => {return i - start});
            falling_edge = falling_edge.map((i) => {return i - start});
            this.draw_data(data.subarray(start*this.channel_count, end*this.channel_count), rising_edge, falling_edge);
            this.output_trigger_data(data, rising_edge, falling_edge);
            this.disarm();
        }

    }

    output_trigger_data(data, rising_edge, falling_edge) {
        let data_str = "";
        let exposures = new Array(this.channel_count);
        for (let i = 0; i < this.channel_count; i++) {
            let start = rising_edge[i]/this.sample_rate;
            let end = falling_edge[i]/this.sample_rate;
            exposures[i] = end - start;
            data_str += `Channel ${i} exposure time: 1/${Math.round(1/exposures[i])}<br/>`;
        }
        this.trigger_data_div.innerHTML = data_str;
    }

}