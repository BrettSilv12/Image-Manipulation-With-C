echo "*----------------------Testing 40image-----------------------*"
make ppmdiff
make 40image
echo "*----------------------Part 1 ppmdiff tests------------------*"
echo "1. /comp/40/bin/images/flowers.ppm"
./40image -c /comp/40/bin/images/flowers.ppm > ./data/compressed/flowers.ppm
./40image -d ./data/compressed/flowers.ppm > ./data/decompressed/flowers.ppm
./ppmdiff ./data/decompressed/flowers.ppm /comp/40/bin/images/flowers.ppm
echo "2. /comp/40/bin/images/flowers_new.ppm"
./40image -c /comp/40/bin/images/flowers_new.ppm > ./data/compressed/flowers_new.ppm
./40image -d ./data/compressed/flowers_new.ppm > ./data/decompressed/flowers_new.ppm
./ppmdiff ./data/decompressed/flowers_new.ppm /comp/40/bin/images/flowers_new.ppm
echo "3. .data/tony.ppm"
./40image -c ./data/tony.ppm > ./data/compressed/tony.ppm
./40image -d ./data/compressed/tony.ppm > ./data/decompressed/tony.ppm
./ppmdiff ./data/tony.ppm ./data/decompressed/tony.ppm
# echo "3. /comp/40/bin/images/large/mobo.ppm"
# ./40image -c /comp/40/bin/images/large/mobo.ppm > ./data/compressed/mobo.ppm
# ./40image -d ./data/compressed/mobo.ppm > ./data/decompressed/mobo.ppm
# ./ppmdiff ./data/decompressed/mobo.ppm /comp/40/bin/images/mobo.ppm

echo "*----------------------Part 2 valgrind tests------------------*"
echo "1. /comp/40/bin/images/flowers.ppm"
valgrind ./40image -c /comp/40/bin/images/flowers.ppm > ./data/compressed/flowers.ppm
valgrind ./40image -d ./data/compressed/flowers.ppm > ./data/decompressed/flowers.ppm
echo "2. /comp/40/bin/images/flowers_new.ppm"
valgrind ./40image -c /comp/40/bin/images/flowers_new.ppm > ./data/compressed/flowers_new.ppm
valgrind ./40image -d ./data/compressed/flowers_new.ppm > ./data/decompressed/flowers_new.ppm
echo "3. ./data/tony.ppm"
valgrind ./40image -c ./data/tony.ppm > ./data/compressed/tony.ppm
valgrind ./40image -d ./data/compressed/tony.ppm > ./data/decompressed/tony.ppm
# echo "3. /comp/40/bin/images/large/mobo.ppm"
# valgrind ./40image -c /comp/40/bin/images/large/mobo.ppm > ./data/compressed/mobo.ppm
# valgrind ./40image -d ./data/compressed/mobo.ppm > ./data/decompressed/mobo.ppm