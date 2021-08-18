cd ../Vortex-Engine/vendor && git clone https://github.com/fmtlib/fmt
git clone https://github.com/kcat/openal-soft
rm -rf OpenAL
mv openal-soft OpenAL
cd ../../build
cmake ..
make
