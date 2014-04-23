all: 
	gcc sushi.c -o sushi -lpthread 

clean:
	rm -rf *o sushi