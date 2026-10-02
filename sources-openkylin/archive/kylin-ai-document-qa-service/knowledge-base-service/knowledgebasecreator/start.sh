mkdir build
cd build
cmake ..;make -j
cd ..

python3 ./file_process.py

systemctl --user restart kylin-ai-vector-engine.service

./build/faqknowledgebasecreator
./build/ragknowledgebasecreator