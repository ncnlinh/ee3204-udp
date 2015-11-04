for VARIABLE1 in {0..10}
do
  if [ $VARIABLE1 = 0 ]; then
    PACKLEN=20
  else
    PACKLEN=$(expr $VARIABLE1 \* 100)
  fi
  replace "#define DATALEN 500" "#define DATALEN $PACKLEN" -- ./headsock.h
  for CORRUPTION in {0..9}
  do 
    CORRATE=$(expr $CORRUPTION \*  10)
    replace "#define CORRUPTED_ACK_RATE 0" "#define CORRUPTED_ACK_RATE $CORRATE" -- ./headsock.h
    for VARIABLE in {1..20}
    do
      gcc ./udp_client4.c -o udp_client4
      ./udp_client4 172.23.194.40
      sleep 1
    done
    replace "#define CORRUPTED_ACK_RATE $CORRATE" "#define CORRUPTED_ACK_RATE 0" -- ./headsock.h
  done
  replace "#define DATALEN $PACKLEN" "#define DATALEN 500" -- ./headsock.h
done