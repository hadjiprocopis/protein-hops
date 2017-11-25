#ifndef _GENERAL_CPU_TIME_MONITOR_HPP_
#define	_GENERAL_CPU_TIME_MONITOR_HPP_
#include <sys/time.h>
#include <sys/resource.h>

class	GeneralCPUTimeMonitor { 
	private:
		typedef	struct timeval	timeval_t;
		typedef	struct rusage	rusage_t;
		rusage_t	rusage_info;
		timeval_t	last_user, last_system,
				current_user, current_system,
				diff_user, diff_system;
		static const size_t	sizeof_timeval = sizeof(timeval_t);
		void	copy(timeval_t *dest, timeval_t *src){
			*dest = *src;
//			memcpy(dest, src, GeneralCPUTimeMonitor::sizeof_timeval);
		}
		void	calculate_difference(void){
			diff_user.tv_sec = current_user.tv_sec - last_user.tv_sec;
			diff_user.tv_usec = current_user.tv_usec - last_user.tv_usec;
			if( diff_user.tv_usec < 0 ){ diff_user.tv_usec += 1000000L; diff_user.tv_sec--; }
			diff_system.tv_sec = current_system.tv_sec - last_system.tv_sec;
			diff_system.tv_usec = current_system.tv_usec - last_system.tv_usec;
			if( diff_system.tv_usec < 0 ){ diff_system.tv_usec += 1000000; diff_system.tv_sec--; }
		}
	public:
		GeneralCPUTimeMonitor(){ this->reset(); }
		void	reset(void){
			getrusage(RUSAGE_SELF, &rusage_info);
			copy(&(this->current_user), &(rusage_info.ru_utime)); copy(&(this->current_system), &(rusage_info.ru_stime));
			copy(&(this->last_user), &(rusage_info.ru_utime)); copy(&(this->last_system), &(rusage_info.ru_stime));
		}
		void	record(void){
			getrusage(RUSAGE_SELF, &rusage_info);
			copy(&(this->current_user), &(rusage_info.ru_utime)); copy(&(this->current_system), &(rusage_info.ru_stime));
			this->calculate_difference();
		}
		
		void	print(FILE *fh){
			fprintf(fh, "user: %ld.%06ld, system: %ld.%06ld (%ld.%06ld, %ld.%06ld)",
				(long int)this->diff_user.tv_sec, (long int)this->diff_user.tv_usec,
				(long int)this->diff_system.tv_sec, (long int)this->diff_system.tv_usec,
				(long int)this->current_user.tv_sec, (long int)this->current_user.tv_usec,
				(long int)this->current_system.tv_sec, (long int)this->current_system.tv_usec
			);
		}
};
#endif
