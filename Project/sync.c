#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

struct RingBuf { int* data; int cap; int head; int tail; int size; pthread_mutex_t m; pthread_cond_t cv_full; pthread_cond_t cv_empty; int stop; int produced; int consumed; };

static void* producer_thread(void* arg){
    struct RingBuf* rb=(struct RingBuf*)arg; unsigned int seed=(unsigned int)pthread_self();
    while(1){
        pthread_mutex_lock(&rb->m);
        while(rb->size==rb->cap && !rb->stop) pthread_cond_wait(&rb->cv_full,&rb->m);
        if(rb->stop){ pthread_mutex_unlock(&rb->m); break; }
        int v=rand_r(&seed)%1000;
        rb->data[rb->tail]=v; rb->tail=(rb->tail+1)%rb->cap; rb->size++; rb->produced++;
        pthread_cond_signal(&rb->cv_empty);
        pthread_mutex_unlock(&rb->m);
        usleep(10000);
    }
    return NULL;
}

static void* consumer_thread(void* arg){
    struct RingBuf* rb=(struct RingBuf*)arg;
    while(1){
        pthread_mutex_lock(&rb->m);
        while(rb->size==0 && !rb->stop) pthread_cond_wait(&rb->cv_empty,&rb->m);
        if(rb->stop && rb->size==0){ pthread_mutex_unlock(&rb->m); break; }
        int v=rb->data[rb->head]; (void)v; rb->head=(rb->head+1)%rb->cap; rb->size--; rb->consumed++;
        pthread_cond_signal(&rb->cv_full);
        pthread_mutex_unlock(&rb->m);
        usleep(15000);
    }
    return NULL;
}

void run_pc(int buffer_size,int producers,int consumers,int seconds){
    struct RingBuf rb; rb.cap=buffer_size; rb.head=0; rb.tail=0; rb.size=0; rb.stop=0; rb.produced=0; rb.consumed=0;
    rb.data=(int*)malloc(buffer_size*sizeof(int)); pthread_mutex_init(&rb.m,NULL); pthread_cond_init(&rb.cv_full,NULL); pthread_cond_init(&rb.cv_empty,NULL);
    pthread_t* pt=(pthread_t*)malloc(producers*sizeof(pthread_t)); pthread_t* ct=(pthread_t*)malloc(consumers*sizeof(pthread_t));
    for(int i=0;i<producers;i++) pthread_create(&pt[i],NULL,producer_thread,&rb);
    for(int i=0;i<consumers;i++) pthread_create(&ct[i],NULL,consumer_thread,&rb);
    sleep(seconds);
    pthread_mutex_lock(&rb.m); rb.stop=1; pthread_cond_broadcast(&rb.cv_full); pthread_cond_broadcast(&rb.cv_empty); pthread_mutex_unlock(&rb.m);
    for(int i=0;i<producers;i++) pthread_join(pt[i],NULL);
    for(int i=0;i<consumers;i++) pthread_join(ct[i],NULL);
    printf("{\"produced\":%d,\"consumed\":%d,\"buffer_size\":%d,\"producers\":%d,\"consumers\":%d}\n",rb.produced,rb.consumed,buffer_size,producers,consumers);
    pthread_mutex_destroy(&rb.m); pthread_cond_destroy(&rb.cv_full); pthread_cond_destroy(&rb.cv_empty); free(rb.data); free(pt); free(ct);
}

struct DPState { int n; pthread_mutex_t* forks; int* eat; int stop; };

struct DPArg { struct DPState* st; int id; };

static void* philosopher_run(void* arg){
    struct DPArg* a=(struct DPArg*)arg; struct DPState* st=a->st; int id=a->id; int left=id; int right=(id+1)%st->n;
    while(!st->stop){
        usleep(10000);
        if(id%2==0){ pthread_mutex_lock(&st->forks[left]); pthread_mutex_lock(&st->forks[right]); }
        else { pthread_mutex_lock(&st->forks[right]); pthread_mutex_lock(&st->forks[left]); }
        st->eat[id]++;
        usleep(10000);
        pthread_mutex_unlock(&st->forks[left]); pthread_mutex_unlock(&st->forks[right]);
    }
    return NULL;
}

void run_dp(int n,int seconds){
    struct DPState st; st.n=n; st.forks=(pthread_mutex_t*)malloc(n*sizeof(pthread_mutex_t)); st.eat=(int*)calloc(n,sizeof(int)); st.stop=0;
    for(int i=0;i<n;i++) pthread_mutex_init(&st.forks[i],NULL);
    pthread_t* th=(pthread_t*)malloc(n*sizeof(pthread_t)); struct DPArg* args=(struct DPArg*)malloc(n*sizeof(struct DPArg));
    for(int i=0;i<n;i++){ args[i].st=&st; args[i].id=i; pthread_create(&th[i],NULL,philosopher_run,&args[i]); }
    sleep(seconds);
    st.stop=1;
    for(int i=0;i<n;i++) pthread_join(th[i],NULL);
    printf("{\"philosophers\":%d,\"seconds\":%d,\"eat\":[",n,seconds);
    for(int i=0;i<n;i++) printf("%d%s",st.eat[i],(i<n-1?",":""));
    printf("]}\n");
    for(int i=0;i<n;i++) pthread_mutex_destroy(&st.forks[i]); free(st.forks); free(st.eat); free(th); free(args);
}