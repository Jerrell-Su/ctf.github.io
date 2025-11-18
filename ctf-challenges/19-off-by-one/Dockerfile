FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /challenge

COPY challenge.c Makefile ./
RUN make

RUN echo "flag{off_by_one_null_byte_overflow}" > flag.txt

RUN useradd -m ctf
USER ctf

CMD ["./challenge"]
