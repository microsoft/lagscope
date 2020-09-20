FROM gcc

RUN apt-get update && \
    apt-get -y install build-essential cmake

RUN git clone https://github.com/microsoft/lagscope.git

WORKDIR lagscope

RUN ./do-cmake.sh build && \
    ./do-cmake.sh install

FROM fedora

RUN curl https://bootstrap.pypa.io/get-pip.py -o /tmp/get-pip.py && \
    python3 /tmp/get-pip.py && \
    pip3 install matplotlib pandas numpy && \
    dnf install -y python3-tkinter iputils

COPY --from=0 /usr/local/bin/lagscope /bin/
COPY --from=0 /lagscope/src/visualize_data.py /

ENV MPLCONFIGDIR /tmp

CMD ["/bin/bash"]
