# Runner
CLI runner type game. The goal is to provide a platform for learning using a game as a baseline.

This works as is you can run `make runner && ./runner` to start up a game.


# Next
- Create a client/server socket connection system to play the game over a network
- Support many connections from the server to server multiple concurrent games from a single server
- Use this architecture as a platform to practice Machine Learning
  - Genetic Agorithm - requires the server to handle many connections simultaneously to be viable
  - Reinforcement Learning -Does not require a many connections
