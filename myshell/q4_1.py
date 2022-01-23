import threading
import time

def func1(x, y):
    for i in range(x, y):
        print(i, end=' ')
    time.sleep(10)

t1 = threading.Thread(target=func1, args=(15,20))
t1.start()
t1.join(5) # block
