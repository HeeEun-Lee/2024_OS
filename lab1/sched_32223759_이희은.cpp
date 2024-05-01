/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32223759
*	    Student name : 이희은
*/

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include "sched.h"

class SPN : public Scheduler{
    private:
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */
    public:
        SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SPN";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */
        }

        int run() override {
            /*
            * 구현 
            */
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }
            if (current_job_.remain_time == 0) {
                current_job_.completion_time = current_time_;
                end_jobs_.push_back(current_job_);

                if (job_queue_.empty()) return -1;

                std::queue<Job> temp_queue;
                int min_service_time = 101;
                bool choose_min = false;
                while (!job_queue_.empty()) {
                    struct Job tmp = job_queue_.front();
                    job_queue_.pop();
                    if (tmp.service_time < min_service_time && tmp.arrival_time < current_time_) {
                        min_service_time = tmp.service_time;
                    }
                    temp_queue.push(tmp);
                }
                
                while (!temp_queue.empty()) {
                    Job tmp = temp_queue.front();
                    temp_queue.pop();
                    if (tmp.service_time == min_service_time && !choose_min) {
                        current_job_ = tmp;
                        choose_min = true;
                        continue;
                    }
                    job_queue_.push(tmp);
                }

                current_time_ += switch_time_;
            }

            if (current_job_.service_time == current_job_.remain_time) {
                current_job_.first_run_time = current_time_;
            }

            current_time_++;
            current_job_.remain_time--;

            return current_job_.name;
        }
};

class RR : public Scheduler{
    private:
        int time_slice_;
        int left_slice_;
        std::queue<Job> waiting_queue;
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */
    public:
        RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead) {
            name = "RR_"+std::to_string(time_slice);
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
            time_slice_ = time_slice; 
            left_slice_ = time_slice;
        }

        int run() override {
            //if1
            //job_queue.front의 arrival time <= current time일 때 waiting queue에 작업 삽입
            if(!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_){
                waiting_queue.push(job_queue_.front());
                job_queue_.pop();
            }

            //if2
            //할당된 작업이 없고, waiting_queue가 비어있지 않으면 작업 할당
            if(current_job_.name == 0 && !waiting_queue.empty()){
                current_job_ = waiting_queue.front();
                waiting_queue.pop();
            }


            //if3
            // 현재 작업이 모두 완료되면
            if(current_job_.remain_time == 0){
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (job_queue_.empty() && waiting_queue.empty()) return -1;
                
                // 새로운 작업 할당
                current_job_ = waiting_queue.front();
                waiting_queue.pop();
                left_slice_ = time_slice_;
                // context switch 타임 추가
                current_time_ += switch_time_;
            }
            //if4
            //left_slice == 0 -> 기존 작업은 waiting queue에 push하고, 다음 작업 할당, waiting_queue에 아무것도 없다면 현재 작업의 left_slice 초기화
            if(left_slice_ == 0){
                if(waiting_queue.empty()){
                    left_slice_ = time_slice_;
                } else {
                    waiting_queue.push(current_job_);
                    current_job_ = waiting_queue.front();
                    waiting_queue.pop(); 
                    left_slice_ = time_slice_;
                    current_time_ += switch_time_;
                }
            }

            //if5
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time){
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;
            //left_slice_ --
            left_slice_--;

            // 스케줄링할 작업명 반환
            return current_job_.name;
        }               
};

class SRT : public Scheduler{
    private:
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */
       
        struct compare
        {
            /* data */
            bool operator()(Job &a, Job &b){
                if(a.remain_time == b.remain_time){
                    return a.arrival_time > b.arrival_time;
                }
                return a.remain_time > b.remain_time;
            }
        };
        std::priority_queue<Job, std::vector<Job>, compare> ready_queue;

    public:
        SRT(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SRT";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */
        }

        int run() override {
            /*
            * 구현 
            */
            //if1
            //job_queue.front의 arrival time <= current time일 때 ready_queue에 작업 삽입
            if(!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_){
                ready_queue.push(job_queue_.front());
                job_queue_.pop();
            }

            //if2
            //할당된 작업이 없고, ready_queue가 비어있지 않으면 작업 할당
            if(current_job_.name == 0 && !ready_queue.empty()){
                current_job_ = ready_queue.top();
                ready_queue.pop();
            }

            // 현재 작업이 모두 완료되면
            if(current_job_.remain_time == 0){
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (job_queue_.empty() && ready_queue.empty()) return -1;
                
                // 새로운 작업 할당
                current_job_ = ready_queue.top();
                ready_queue.pop();
                // context switch 타임 추가
                current_time_ += switch_time_;
            }

            if(current_job_.remain_time > ready_queue.top().remain_time){
                // 기존 작업 ready_queue에 넣기
                ready_queue.push(current_job_);
                // 새로운 작업 할당
                current_job_ = ready_queue.top();
                ready_queue.pop();
                // context switch 타임 추가
                current_time_ += switch_time_;
            }

            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time){
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
                //std::cout << "Current time: " << current_time_ << ", current Job: " << current_job_.name << '\n';
            }

            

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;

            // 스케줄링할 작업명 반환
            return current_job_.name;


        }
};

class HRRN : public Scheduler{
    private:
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */    
       struct compare
        {
            double current_time = 0;
            /* data */
            bool operator()(Job &a, Job &b){
                // 응답률 = (대기시간 + 서비스 시간)/서비스 시간
                double a_ratio = ((current_time - a.arrival_time + a.service_time)/a.service_time);
                double b_ratio = ((current_time - b.arrival_time + b.service_time)/b.service_time);
                if(a_ratio == b_ratio){
                    return a.arrival_time > b.arrival_time;
                }
                return a_ratio < b_ratio;
            }
        };
        std::vector<Job> jobs;
    public:
        HRRN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "HRRN";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */            
        }

        int run() override {
            /*
            구현 
            */
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if(!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_){
                jobs.push_back(job_queue_.front());
                job_queue_.pop();
            }
            compare cmp;
            cmp.current_time = current_time_;
            std::sort(jobs.begin(), jobs.end(), cmp);
            if (current_job_.name == 0 && !jobs.empty()){
                current_job_ = jobs.back();
                jobs.pop_back();
            }
            // 현재 작업이 모두 완료되면
            if(current_job_.remain_time == 0){
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (job_queue_.empty() && jobs.empty()) return -1;
                
                // 새로운 작업 할당
                current_job_ = jobs.back();
                jobs.pop_back();
                // context switch 타임 추가
                current_time_ += switch_time_;
            }
            
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time){
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;

            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};

// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler{
    private:
        // std::queue<Job> queue_1;
        // std::queue<Job> queue_2;
        // std::queue<Job> queue_3;
        // std::queue<Job> queue_4;
        std::vector<std::queue<Job>> queues;
        std::vector<int> time_slices;
        int left_slice_;
        int priority;

        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */    
    public:
        FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
            if(is_2i){
                name = "FeedBack_2i";
            } else {
                name = "FeedBack_1";
            }
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */
            if(is_2i){
                // 각 큐의 time_slice_ 설정
                time_slices.push_back(1);
                time_slices.push_back(2);
                time_slices.push_back(4);
                time_slices.push_back(8);
            } else {
                time_slices.push_back(1);
                time_slices.push_back(1);
                time_slices.push_back(1);
                time_slices.push_back(1);
            }
            queues.resize(4);
        }

        bool all_empty(){
            for(int i = 0; i < queues.size(); i++){
                if(!queues[i].empty()){
                    return false;
                }
            }
            return true;
        }
 
        int run() override {
            // job_queue가 비어있지 않고 도착시간이 현재시간보다 작을 때 첫번째 큐에 넣음
            if(!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_){
                queues[0].push(job_queue_.front());
                // all_empty = false;
                job_queue_.pop();
            }
            // std::cout << current_time_ << " " << all_empty() << "\n";
            //if2
            //할당된 작업이 없고, waiting_queue가 비어있지 않으면 작업 할당
            if(current_job_.name == 0 && !all_empty()){
                current_job_ = queues[0].front();
                queues[0].pop();
                left_slice_ = time_slices[0];
                priority = 0;
            }

            //if3
            // 현재 작업이 모두 완료되면
            if(current_job_.remain_time == 0){
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (job_queue_.empty() && all_empty()) return -1;
                
                // 새로운 작업 할당
                if(!all_empty()){
                    for(int i = 0; i < queues.size(); i++){
                        if(!queues[i].empty()){
                            current_job_ = queues[i].front();
                            left_slice_ = time_slices[i];
                            queues[i].pop();
                            priority = i;
                            break;
                        }
                    }
                }
                // context switch 타임 추가
                current_time_ += switch_time_;
            }

            //if4
            //left_slice == 0 -> 기존 작업은 waiting queue에 push하고, 다음 작업 할당, waiting_queue에 아무것도 없다면 현재 작업의 left_slice 초기화
            if(left_slice_ == 0){
                if(priority == 3 || all_empty()){
                    queues[priority].push(current_job_);
                } else {
                    queues[priority + 1].push(current_job_);
                }
                for(int i = 0; i < queues.size(); i++){
                    if(!queues[i].empty()){
                        if(current_job_.name != queues[i].front().name){
                            current_time_ += switch_time_;
                        }
                        current_job_ = queues[i].front();
                        left_slice_ = time_slices[i];
                        queues[i].pop();
                        priority = i;
                        break;
                    }
                }

            }

            //if5
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time){
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;
            //left_slice_ --
            left_slice_--;

            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};
