FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install pwntools

WORKDIR /challenge

COPY challenge.c Makefile ./
RUN make

RUN echo "flag{race_condition_toctou_double_spend}" > flag.txt

RUN useradd -m ctf
USER ctf

CMD ["./challenge"]
