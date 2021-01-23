./exec.sh

(./receiver.o -p 11 & ./sender.o -p 10 ../tests/t.pdf) && diff ../tests/t.pdf ../tests/t.pdf_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ../tests/p.gif) && diff ../tests/p.gif ../tests/p.gif_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ../tests/t.hex) && diff ../tests/t.hex ../tests/t.hex_ && echo 'Done'
