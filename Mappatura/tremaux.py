from multiprocessing import Queue
#import pygame
import random
import config


#lista queue = [0 muro dx, 1 muro sx, 2 muro avanti, 3 muro dietro,4 casella(colore), 5 vittima, 6 direzione]

maze_width = config.WIDTH #config maze width
maze_height = config.HEIGHT #config maze height
tile_size = config.TILE_SIZE    #config tile size

tx = Queue()    #init tx queue
tx1 = Queue()
rx = Queue()    #init rx queue
lack = Queue()  #init lack queue

class Cell:
    def __init__(self, x, y, screen):
        self.x = x  #x position of Cell
        self.y = y  #y position of Cell
        self.walls = {  #walls of each Cell
            "top": True,
            "right": True,
            "bottom": True,
            "left": True
        }
        self.visited = False    #mark if Cell is visited
        self.screen = screen
        self.color = 'white'    #Mark each color Cell(white, black, blue)
        self.victim = False     #Mark if Cell has victim

    '''def draw_cell(self):
        pos_x, pos_y = self.x*config.TILE_SIZE, self.y*config.TILE_SIZE
        endpoint = (self.x == (config.WIDTH//config.TILE_SIZE)-1) and (self.y == (config.HEIGHT//config.TILE_SIZE)-1)

        if endpoint:
            cell_color = (0, 255, 0)  # Green color for the endpoint
        if endpoint:
            cell_color = (0, 255, 0)  # Green color for the endpoint
        elif self.visited:
            cell_color = (0, 0, 0)  # Black color for visited cells
        else:
            cell_color = (255, 255, 255)  # Default color for unvisited cells

        pygame.draw.rect(
            self.screen,
            cell_color,
            (pos_x, pos_y, config.TILE_SIZE, config.TILE_SIZE)
        )
        wall_color = (255, 255, 255)
        wall_thickness = 2
        if self.walls['top']:
            pygame.draw.line(
                self.screen,
                wall_color,
                (pos_x, pos_y),
                (pos_x+config.TILE_SIZE, pos_y),
                wall_thickness
            )
        if self.walls['right']:
            pygame.draw.line(
                self.screen,
                wall_color,
                (pos_x+config.TILE_SIZE, pos_y),
                (pos_x+config.TILE_SIZE, pos_y+config.TILE_SIZE),
                wall_thickness
            )
        if self.walls['bottom']:
            pygame.draw.line(
                self.screen,
                wall_color,
                (pos_x+config.TILE_SIZE, pos_y+config.TILE_SIZE),
                (pos_x, pos_y+config.TILE_SIZE),
                wall_thickness
            )
        if self.walls['left']:
            pygame.draw.line(
                self.screen,
                wall_color,
                (pos_x, pos_y+config.TILE_SIZE),
                (pos_x, pos_y),
                wall_thickness
            )'''


class Maze:
    def __init__(self, height, width, tile_size, screen):
        self.height = height    #Maze height
        self.width = width      #Maze width
        self.tile_size = tile_size  #Maze tale size
        self.screen = screen    

        self.rows, self.cols = self.height//self.tile_size, self.width//self.tile_size
        self.grid_cells = [
            Cell(row, col, self.screen) for row in range(self.rows) for col in range(self.cols) #Init each Maze's Cell
        ]
        self.current_cell = self.grid_cells[18+18*self.cols] #[x+y*self.cols]   #start Cell (Centre maze)
        self.current_cell.visited = True
        self.visited_stack = [self.current_cell] #put in a list each visited Cell
        self.path = []  #put in a list path
        self.silver = 0 #Mark how silver found
        self.from_silver = 0    #Mark how Cell there are from last silver Cell
        self.restart = self.grid_cells[18+18*self.cols] #Restart in case of lack of progress and univisited silver Cell
        self.direction = 0

    '''def draw_current_cell(self):
        cur_x = self.current_cell.x*self.tile_size
        cur_y = self.current_cell.y*self.tile_size
        pygame.draw.rect(
            self.screen,
            (255, 128, 0),
            (cur_x, cur_y, self.tile_size, self.tile_size)
        )'''
    
    '''def draw_maze(self):
        for cell in self.grid_cells:
            cell.draw_cell()'''
    
    def Tremaux(self):
        self.read_queue(self.current_cell) #read of queue list
        self.path.append(self.current_cell) #append start cell in path
        while not self.is_exit(self.current_cell): #While is not exit
            unvisited_neighbors = self.get_unvisited_neighbors(self.current_cell) #take univisited neighbors
            print(self.current_cell.x, self.current_cell.y) #print current Cell
            if unvisited_neighbors: #If there are univisited neighbors
                tx1.put(0)#Guidance disable
                #next_cell = random.choice(unvisited_neighbors) #Choose a random univisited Cell
                #tx.put(self.get_direction(next_cell, self.current_cell)) #Put in queue direction
                #print(self.get_direction(next_cell, self.current_cell)) #print direction
                while rx.empty(): #Wait until queue is not empty
                    pass
                self.read_queue(self.current_cell) #read of queue list
                self.current_cell = self.next_cell()
                self.visited_stack.append(self.current_cell) #put in visited stack next Cell
                self.current_cell.visited = True #Mark as visited current Cell
                self.path.append(self.current_cell) #Append in path next cell
                #self.current_cell = next_cell #current cell becomes next cell
                #item = get_input()
                #rx.put(item)
                self.black_control() #Black control
                self.silver_control()   #silver control
                if self.lack_control(): #lack control
                    self.got_lack() #if lack = True
                
                #self.draw_maze()
                #self.draw_current_cell()
            else: #If not univisited neighbors
                tx1.put(1)#Guidance enabled
                self.path.pop() #pop from path current cell
                if not self.path:
                    print('no path') #if here returned Start Cell (maybe)
                    break
                tx.put(self.get_direction(self.path[-1], self.current_cell)) #Put in queue direction
                print(self.get_direction(self.path[-1], self.current_cell)) #print direction
                self.current_cell = self.path[-1] #current Cell becomes previous Cell
        return self.path
    
    def get_direction(self, next_node, current_node):
        direction = 0
        if next_node.x == current_node.x and next_node.y == current_node.y + 1:
            direction = 2
        if next_node.x == current_node.x and next_node.y == current_node.y - 1:
            direction = 0
        if next_node.x == current_node.x + 1 and next_node.y == current_node.y:
            direction = 1
        if next_node.x == current_node.x - 1 and next_node.y == current_node.y:
            direction = 3
        return direction
    
    def next_cell(self):
        match self.direction:
            case 0:
                return self.grid_cells[self.current_cell.x + (self.current_cell.y - 1)*self.cols]
            case 1:
                return self.grid_cells[(self.current_cell.x + 1) + self.current_cell.y*self.cols]
            case 2:
                return self.grid_cells[self.current_cell.x + (self.current_cell.y + 1)*self.cols]
            case 3:
                return self.grid_cells[(self.current_cell.x - 1) + self.current_cell.y*self.cols]

    def get_unvisited_neighbors(self, cell):
        def get_valid_neighbors():
            col, row = cell.x, cell.y
            walls = cell.walls

            def get_cell(x, y):
                if 0 <= x < self.cols and 0 <= y < self.rows:
                    return self.grid_cells[x + y * self.cols]
                return None
            
            def valid_neighbor(neighbor):
                # checks if there is no wall between cell and neighbor
                if neighbor.y == row-1:
                    return not cell.walls['top']
                if neighbor.x == col+1:
                    return not cell.walls['right']
                if neighbor.y == row+1:
                    return not cell.walls['bottom']
                if neighbor.x == col-1:
                    return not cell.walls['left']
                return False
            
            neighbors = [get_cell(row-1, col), get_cell(row, col+1), get_cell(row+1, col), get_cell(row, col-1)]
            return [neighbor for neighbor in neighbors if neighbor and valid_neighbor(neighbor)]

        neighbors = get_valid_neighbors()
        return [n for n in neighbors if n not in self.visited_stack]


    def is_exit(self, cell):
        return cell.x == self.cols and cell.y-1 == self.rows-1
    
    def read_queue(self):
        queue = []
        queue = rx.get()
        self.direction = queue[6]
        #direction's control
        match self.direction:
            case 0: 
                if queue[0] == 0:
                    self.current_cell.walls['right'] = False
                if queue[1] == 0:
                    self.current_cell.walls['left'] = False
                if queue[2] == 0:
                    self.current_cell.walls['top'] = False
                if queue[3] == 0:
                    self.current_cell.walls['bottom'] = False
            case 1:
                if queue[0] == 0:
                    self.current_cell.walls['bottom'] = False
                if queue[1] == 0:
                    self.current_cell.walls['top'] = False
                if queue[2] == 0:
                    self.current_cell.walls['right'] = False
                if queue[3] == 0:
                    self.current_cell.walls['left'] = False
            case 2:
                if queue[0] == 0:
                    self.current_cell.walls['left'] = False
                if queue[1] == 0:
                    self.current_cell.walls['right'] = False
                if queue[2] == 0:
                    self.current_cell.walls['bottom'] = False
                if queue[3] == 0:
                    self.current_cell.walls['top'] = False
            case 3:
                if queue[0] == 0:
                    self.current_cell.walls['top'] = False
                if queue[1] == 0:
                    self.current_cell.walls['bottom'] = False
                if queue[2] == 0:
                    self.current_cell.walls['left'] = False
                if queue[3] == 0:
                    self.current_cell.walls['right'] = False
        #color's control
        match queue[4]:
            case 'white':
                self.current_cell.color = 'white'
            case 'black':
                self.current_cell.color = 'black'
            case 'blue':
                self.current_cell.color = 'blue'
            case 'silver':
                self.current_cell.color = 'silver'
        #victim's control
        if queue[5] == True:
            self.current_cell.victim = True
        else:
            self.current_cell.victim = False
    
    def black_control(self):
        if self.current_cell.color == 'black': #If current Cell put all 4 walls in that Cell
            self.current_cell.walls['top'] = True
            self.current_cell.walls['right'] = True
            self.current_cell.walls['left'] = True
            self.current_cell.walls['bottom'] = True
            self.current_cell.visited = True
            self.visited_stack.append(self.current_cell)
            self.path.pop()
            self.current_cell = self.path[-1] #current Cell becomes Black's Previous Cell
            match self.direction: #Put a wall in Black direction
                case 0:
                    self.current_cell.walls['top'] = True
                case 1:
                    self.current_cell.walls['left'] = True
                case 2:
                    self.current_cell.walls['bottom'] = True
                case 3:
                    self.current_cell.walls['right'] = True
            return True
        else: 
            return False
    
    def silver_control(self):
        if self.current_cell.color == 'silver':
            self.restart = self.current_cell
            self.silver += 1
            self.from_silver = -1 #maybe =0
        if self.silver > 0:
            self.from_silver += 1

    def lack_control(self):
        get_lack_queue = 0
        if lack.qsize() > 0:
            get_lack_queue = lack.get()
            return True
        else:
            return False

    def got_lack(self): #To try, not sure if works
        for i in range(self.from_silver):
            self.current_cell.visited = self.visited_stack[len(self.visited_stack) - i] = False
            self.visited_stack.remove[len(self.visited_stack) - i]
            self.path.remove[len(self.path) - i]
        self.current_cell = self.restart
    
        

'''def get_input(): #Get in input values of rx queue
    input_list = []
    for i in range(4):
        item = int(input('laser' + str(i) + ': '))
        input_list.append(item)
    item = input('colore: ')
    input_list.append(item)
    for i in range(2):
        item = int(input('salita e direzione:'))
        input_list.append(item)
    print(input_list)
    return input_list'''


def main():
    screen = 1#pygame.display.set_mode((maze_height,maze_width), 0, 32) #Surface of pygame
    maze = Maze(maze_height, maze_width, tile_size, screen) #Set Maze
    while rx.qsize() == 0: #First Queue if False go on and di Tremaux algorithm
        pass
    #rx.put(get_input())
    maze.Tremaux() #Tremaux Algorithm
    #pygame.quit()

if __name__ == '__main__': #Main
    main()