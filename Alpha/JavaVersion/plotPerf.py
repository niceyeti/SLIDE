import matplotlib.pyplot as plt

ranks = [float(token) for token in open("perfIts.txt","r").readlines()[0].split(",") if len(token.strip()) > 0]
xs = [i for i in range(len(ranks))]

plt.plot(xs, ranks, color="blue")
plt.savefig("trainingPerf.png")
plt.show()






