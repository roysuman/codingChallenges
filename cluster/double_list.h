
/*
 * =====================================================================================
 *
 *       Filename:  double_list.h
 *
 *       Description:  
 *
 *       Version:  1.0
 *       Created:  Saturday 27 February 2016 11:42:28  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *       Author:  SIGCONT (suman roy), email.suman.roy@gmail.com
 *       Organization:  OPEN SOURCE
 *       LICENSE: GNU GPL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * =====================================================================================
 */



#ifndef LIST_H
#define LIST_H
#include<stddef.h>
#include<iostream>
extern "C" {
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)
typedef struct list_head_ list_head;
struct list_head_{
	list_head *next;
	list_head *prev;
};
#define INIT_LIST_HEAD(list_ptr)\
	do{\
		(list_ptr)->next = ( list_ptr);\
		(list_ptr)->prev = (list_ptr );\
	}while(0)

#define list_for_each(pos,head)\
	for ( pos = (head)->next; pos != head ; pos =(pos)->next)

#define list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                pos = n, n = pos->next)
#define ist_for_each_prev(pos,n,head)	\
	for ( pos = n ; n= pos->prev;pos!=(head);	\
			pos =n,n=pos->prev

#define get_member_struct(ptr,type,struct_member)({     \
	decltype( ( (type *)0)->struct_member) *__mptr = (ptr);	\
	(type *)( (char*)__mptr - (( (size_t) &((type*)0)->struct_member) ));})
	/* 
	typeof &( ( (type *)0)->struct_member) *__mptr = (ptr);     \
	                (type *)( (char*)__mptr - (( (size_t) &((type*)0)->struct_member) ));})
*/

	static inline void 
		__add_list__( list_head *new_,
			      list_head *prev,
			      list_head *next){
			next->prev= new_;
			new_->next = next;
			new_->prev= prev;
			prev->next = new_;
			return;
		}
	static inline void 
		list_add_tail( list_head *new_,
			       list_head *head){
			__add_list__(new_,head->prev,head);
			return;
		}
	static inline void 
		__delete_list__( list_head *next,
				 list_head *prev ){
			prev->next = next;
			next->prev = prev;
			return;
		}
	static inline void 
		delete_list( list_head *entry){
			__delete_list__(entry->next , entry->prev);
			entry->next = (list_head*)LIST_POISON1;
			entry->prev = (list_head*)LIST_POISON2;
			return;
		}
	
}
#endif
