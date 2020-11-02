./exec.sh

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/d/t.pdf) && diff ./tests/d/t.pdf ./tests/d/t.pdf_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/f/f.jpg) && diff ./tests/f/f.jpg ./tests/f/f.jpg_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/f/f.png) && diff ./tests/f/f.png ./tests/f/f.png_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/f/g.jpg) && diff ./tests/f/g.jpg ./tests/f/g.jpg_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/f/i.jpg) && diff ./tests/f/i.jpg ./tests/f/i.jpg_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/p/p.gif) && diff ./tests/p/p.gif ./tests/p/p.gif_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/t/t.hex) && diff ./tests/t/t.hex ./tests/t/t.hex_ && echo 'Done'

(./receiver.o -p 11 & ./sender.o -p 10 ./tests/v/v.mkv) && diff ./tests/v/v.mkv ./tests/v/v.mkv_ && echo 'Done'