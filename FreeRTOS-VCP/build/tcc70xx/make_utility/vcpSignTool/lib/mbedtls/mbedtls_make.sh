
cd source
sed -i "s/^\/\/#define MBEDTLS_CMAC_C/#define MBEDTLS_CMAC_C/g" include/mbedtls/config.h
make

# Library file copy
cp library/libmbedx509.a ../
cp library/libmbedcrypto.a ../
cp library/libmbedtls.a ../

# Include file copy
rm -rf ../include
mkdir ../include
cp -rf ./include/mbedtls ../include

