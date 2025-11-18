FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /challenge

COPY challenge.c Makefile ./
RUN make

RUN echo "flag{stack_pivot_to_bss_segment}" > flag.txt

RUN useradd -m ctf
USER ctf

CMD ["./challenge"]
