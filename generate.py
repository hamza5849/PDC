import random
words = ["the","quick","computer","science","parallel","distributed","master","worker","map","reduce","thread","core","speed","network","data","process","memory","system"]
f = open("input.txt","w")
for i in range(100000):
    f.write(random.choice(words)+" ")
f.close()
print("done")