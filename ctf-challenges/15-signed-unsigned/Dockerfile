FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /challenge

COPY challenge.c Makefile ./
RUN make

RUN echo "flag{signed_unsigned_type_confusion}" > flag.txt

RUN useradd -m ctf
USER ctf

CMD ["./challenge"]
