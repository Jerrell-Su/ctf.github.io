FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libc6-dbg \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /challenge

COPY challenge.c Makefile ./
RUN make

RUN echo "flag{heap_overflow_function_pointer_overwrite}" > flag.txt

RUN useradd -m ctf
USER ctf

CMD ["./challenge"]
