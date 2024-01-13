#!/bin/bash

# Check if domain argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <domain>"
    exit 1
fi

DOMAIN=$1

# Create temporary files
temp_cert_pem=$(mktemp)
temp_cert_der=$(mktemp)

# Replace dots in the domain name with underscores for the output file name
filename_safe_domain="${DOMAIN//./_}"
output_filename="${filename_safe_domain}_trust.h"
#var_name="cert_${filename_safe_domain}"
#guard_macro="${filename_safe_domain}_H_"

# Function to cleanup temporary files
cleanup() {
  rm -f "$temp_cert_pem" "$temp_cert_der"
}

# Ensure temporary files are deleted on script exit
trap cleanup EXIT

# Fetch the certificate in PEM format
openssl s_client -showcerts -connect ${DOMAIN}:443 </dev/null | sed -n -e '/-.BEGIN/,/-.END/ p' > "$temp_cert_pem"

# Check if brssl successfully retrieved the certificate
if [ $? -ne 0 ]; then
    echo "Failed to fetch the certificate for ${DOMAIN}"
    exit 2
fi

cert_var=$(echo "$filename_safe_domain" | tr '[:lower:]' '[:upper:]') # Convert guard macro to uppercase

# Convert PEM to DER format
./env/bin/python certs/pycert_bearssl.py convert "$temp_cert_pem" --no-search --cert-var ${cert_var}_TAs  \
                                                                  --cert-length-var ${cert_var}_TAs_NUM \
                                                                  --output "$output_filename"
echo $?
#brssl ta "$temp_cert_pem" > "$temp_cert_der"
#brssl chain "$temp_cert_pem" > "_test_$output_filename"
#
## Start writing to the output file
#echo "#ifndef _${guard_macro}" > "$output_filename"
#echo "#define _${guard_macro}" >> "$output_filename"
#echo "" >> "$output_filename"
#echo "#ifdef __cplusplus" >> "$output_filename"
#echo "extern \"C\"" >> "$output_filename"
#echo "{" >> "$output_filename"
#echo "#endif" >> "$output_filename"
#echo "" >> "$output_filename"
#
## Append the generated content from xxd
#xxd -i -n "$var_name" "$temp_cert_der" >> "$output_filename"
#
## Append the footer to the output file
#echo "" >> "$output_filename"
#echo "#ifdef __cplusplus" >> "$output_filename"
#echo "} /* extern \"C\" */" >> "$output_filename"
#echo "#endif" >> "$output_filename"
#echo "" >> "$output_filename"
#echo "#endif /* ifndef _${guard_macro} */" >> "$output_filename"
#
# echo "Generated ${output_filename}"
