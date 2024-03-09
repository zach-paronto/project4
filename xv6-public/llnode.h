void insert_node(struct proc *process, struct node *new);
void insert(struct proc *process, uint addr, int length, int flags, int fd);
void remove(struct proc *process, uint addr);
struct node *find(struct proc *process, uint addr);
struct node *find_free(struct proc *process, int length);
int is_conflict(struct proc *process, uint addr, int length);
void free_all(struct proc *process);