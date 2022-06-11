FROM wiiuenv/devkitppc:20220605

RUN git clone --depth 1 --single-branch -b filesystemstructs https://github.com/Maschell/wut && cd wut && git reset --hard ad9e9c06a2ae8e813df296c668a96c06b966e520 && make install && cd .. && rm -rf wut

WORKDIR tmp_build
COPY . .
RUN make clean && make && mkdir -p /artifacts/wut/usr && cp -r lib /artifacts/wut/usr && cp -r include /artifacts/wut/usr
WORKDIR /artifacts

FROM scratch
COPY --from=0 /artifacts /artifacts