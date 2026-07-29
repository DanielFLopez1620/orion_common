mkdir -p V1.3.2_com
for img in V1.3.2/*.jpg; do
    filename=$(basename "$img")
    convert "$img" -quality 20% "V1.3.2_com/$filename"
done
