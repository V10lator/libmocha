FROM wiiuenv/devkitppc:20220605

RUN git clone --depth 1 --single-branch -b filesystemstructs https://github.com/Maschell/wut && cd wut && git reset --hard 1780dc5747a0f5fe708b9bbf5b280fa41f6d40e1 && make install && cd .. && rm -rf wut

WORKDIR tmp_build
COPY . .
RUN make clean && make && mkdir -p /artifacts/wut/usr && cp -r lib /artifacts/wut/usr && cp -r include /artifacts/wut/usr
WORKDIR /artifacts

FROM scratch
COPY --from=0 /artifacts /artifacts