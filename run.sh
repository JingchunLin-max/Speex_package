rm speexenc -f
rm speexdec -f
gcc  speex_api.c speexenc.c -I speex_ogg/speex/include/ -L./speex_ogg/speex/libspeex/.libs -lspeex -o speexenc
gcc  speex_api.c speexdec.c -I speex_ogg/speex/include/ -L./speex_ogg/speex/libspeex/.libs -lspeex -o speexdec
