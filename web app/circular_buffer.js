class CircularBuffer {
    constructor(buffer, allow_overflow = false) {
        this.buffer = buffer;
        this.buffer_length = buffer.length;
        this.write_pointer = 0;
        this.read_pointer = 0
        this.length = 0;
        this.sample_count = 0;
        this.allow_overflow = allow_overflow;
        this.full = false;
    }

    calculate_length() {
        if (this.write_pointer == this.read_pointer && this.full) {
            this.length = this.buffer_length;
        } else if (this.write_pointer >= this.read_pointer) {
            this.length =  this.write_pointer - this.read_pointer;
        } else {
            this.length = this.write_pointer - this.read_pointer + this.buffer_length;
        }
    }

    push(push_array) {
        const push_array_len = push_array.length;
        let increment_read_pointer = false;
        if (!this.allow_overflow && this.buffer_length - this.length < push_array_len) {
            throw "Buffer Overflow, data is being discarded";
        } else if (this.buffer_length - this.length < push_array_len){
            increment_read_pointer = true;
        }
        if (this.buffer_length - this.write_pointer > push_array_len) {
            this.buffer.set(push_array, this.write_pointer);
        } else {
            this.buffer.set(push_array.subarray(0,this.buffer_length - this.write_pointer), this.write_pointer);
            this.buffer.set(push_array.subarray(this.buffer_length - this.write_pointer),0);
        }
        this.write_pointer += push_array_len;
        this.write_pointer %= this.buffer_length;
        if (increment_read_pointer) {
            this.read_pointer = this.write_pointer;
            this.read_pointer %= this.buffer_length;
        }
        if(this.read_pointer == this.write_pointer) {
            this.full = true;
        }
        this.sample_count += push_array_len;
        this.calculate_length();
    }

    pop(pop_array, destructive= true) {
        const pop_array_len = pop_array.length;
        if (this.length < pop_array_len) {
            throw "Buffer Underflow";
        }
        if (this.buffer_length - this.read_pointer > pop_array_len) {
            pop_array.set(this.buffer.subarray(this.read_pointer, this.read_pointer + pop_array_len));
        } else {
            pop_array.set(this.buffer.subarray(this.read_pointer));
            pop_array.set(this.buffer.subarray(0, pop_array_len - (this.buffer_length - this.read_pointer)), this.buffer_length - this.read_pointer);
        }

        if (destructive) {
            this.read_pointer += pop_array_len;
            this.read_pointer %= this.buffer_length;
            this.full = false;
        }
        this.calculate_length();
    }

    clear() {
        this.read_pointer = this.write_pointer = 0;
        this.full = false;
        this.calculate_length();
    }

}