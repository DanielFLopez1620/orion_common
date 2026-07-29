for i in $(seq -w 9 60); do
  step=$((10#$i - 8))
  echo "<div style=\"display: inline-block; text-align: center; margin: 10px;\">"
  echo "  <b>Step ${step}</b>"
  echo "  <p>Info...</p>"
  echo "  <img src=\"https://github.com/Tesis-ORION/orion_common/blob/main/docs/V1.5.1_com/A0${i}.jpg\" width=\"400\"/><br/>"
  echo "</div>"
done
