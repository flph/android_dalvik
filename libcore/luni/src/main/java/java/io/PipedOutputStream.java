/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package java.io;

import org.apache.harmony.luni.util.Msg;

/**
 * Places information on a communications pipe. When two threads want to pass
 * data back and forth, one creates a piped output stream and the other one
 * creates a piped input stream.
 *
 * @see PipedInputStream
 */
public class PipedOutputStream extends OutputStream {

    /**
     * The destination PipedInputStream
     */
    private PipedInputStream dest;

    /**
     * Constructs a new unconnected {@code PipedOutputStream}. The resulting
     * stream must be connected to a {@link PipedInputStream} before data can be
     * written to it.
     */
    public PipedOutputStream() {
        super();
    }

    /**
     * Constructs a new {@code PipedOutputStream} connected to the
     * {@link PipedInputStream} {@code dest}. Any data written to this stream
     * can be read from the target stream.
     *
     * @param dest
     *            the piped input stream to connect to.
     * @throws IOException
     *             if this stream or {@code dest} are already connected.
     */
    public PipedOutputStream(PipedInputStream dest) throws IOException {
        super();
        connect(dest);
    }

    /**
     * Closes this stream. If this stream is connected to an input stream, the
     * input stream is closed and the pipe is disconnected.
     *
     * @throws IOException
     *             if an error occurs while closing this stream.
     */
    @Override
    public void close() throws IOException {
        // Is the pipe connected?
        if (dest != null) {
            dest.done();
            dest = null;
        }
    }

    /**
     * Connects this stream to a {@link PipedInputStream}. Any data written to
     * this output stream becomes readable in the input stream.
     *
     * @param stream
     *            the destination input stream.
     * @throws IOException
     *             if either stream is already connected.
     */
    public void connect(PipedInputStream stream) throws IOException {
        if (null == stream) {
            throw new NullPointerException();
        }
        if (this.dest != null) {
            throw new IOException(Msg.getString("K0079")); //$NON-NLS-1$
        }
        synchronized (stream) {
            if (stream.isConnected) {
                throw new IOException(Msg.getString("K007a")); //$NON-NLS-1$
            }
            stream.buffer = new byte[PipedInputStream.PIPE_SIZE];
            stream.isConnected = true;
            this.dest = stream;
        }
    }

    /**
     * Notifies the readers of this {@link PipedInputStream} that bytes can be
     * read. This method does nothing if this stream is not connected.
     *
     * @throws IOException
     *             if an I/O error occurs while flushing this stream.
     */
    @Override
    public void flush() throws IOException {
        if (dest != null) {
            synchronized (dest) {
                dest.notifyAll();
            }
        }
    }

    /**
     * Writes {@code count} bytes from the byte array {@code buffer} starting at
     * {@code offset} to this stream. The written data can then be read from the
     * connected input stream.
     * <p>
     * Separate threads should be used to write to a {@code PipedOutputStream}
     * and to read from the connected {@link PipedInputStream}. If the same
     * thread is used, a deadlock may occur.
     *
     * @param buffer
     *            the buffer to write.
     * @param offset
     *            the index of the first byte in {@code buffer} to write.
     * @param count
     *            the number of bytes from {@code buffer} to write to this
     *            stream.
     * @throws IndexOutOfBoundsException
     *             if {@code offset < 0} or {@code count < 0}, or if {@code
     *             offset + count} is bigger than the length of {@code buffer}.
     * @throws InterruptedIOException
     *             if the pipe is full and the current thread is interrupted
     *             waiting for space to write data. This case is not currently
     *             handled correctly.
     * @throws IOException
     *             if this stream is not connected, if the target stream is
     *             closed or if the thread reading from the target stream is no
     *             longer alive. This case is currently not handled correctly.
     */
    @Override
    public void write(byte[] buffer, int offset, int count) throws IOException {
        // BEGIN android-note
        // changed array notation to be consistent with the rest of harmony
        // END android-note
        if (dest == null) {
            // K007b=Pipe Not Connected
            throw new IOException(Msg.getString("K007b")); //$NON-NLS-1$
        }
        super.write(buffer, offset, count);
    }

    /**
     * Writes a single byte to this stream. Only the least significant byte of
     * the integer {@code oneByte} is written. The written byte can then be read
     * from the connected input stream.
     * <p>
     * Separate threads should be used to write to a {@code PipedOutputStream}
     * and to read from the connected {@link PipedInputStream}. If the same
     * thread is used, a deadlock may occur.
     *
     * @param oneByte
     *            the byte to write.
     * @throws InterruptedIOException
     *             if the pipe is full and the current thread is interrupted
     *             waiting for space to write data. This case is not currently
     *             handled correctly.
     * @throws IOException
     *             if this stream is not connected, if the target stream is
     *             closed or if the thread reading from the target stream is no
     *             longer alive. This case is currently not handled correctly.
     */
    @Override
    public void write(int oneByte) throws IOException {
        if (dest == null) {
            throw new IOException(Msg.getString("K007b")); //$NON-NLS-1$
        }
        dest.receive(oneByte);
    }
}
