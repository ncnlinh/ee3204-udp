for VARIABLE1 in {0..10}
do
  for CORRUPTION in {0..9}
  do 
    CORRATE=$(expr $CORRUPTION \*  10)
    sed -i "" "s/#define CORRUPTED_ACK_RATE 0/#define CORRUPTED_ACK_RATE $CORRATE/g" headsock.h
    
    for VARIABLE in {1..20}
    do
      ./udp_ser4 &
      wait %1
    done
    sed -i "" "s/#define CORRUPTED_ACK_RATE $CORRATE/#define CORRUPTED_ACK_RATE 0/g" headsock.h
  done
  
done