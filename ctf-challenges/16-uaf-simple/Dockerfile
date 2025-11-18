FROM ubuntu:20.04

ENV GLIBC_TUNABLES=glibc.malloc.tcache_count=7

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libc6-dbg \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /challenge

COPY challenge.c Makefile ./
RUN make

RUN echo "flag{use_after_free_function_pointer}" > flag.txt

RUN useradd -m ctf
USER ctf

CMD ["./challenge"]
