/*
 * request.h
 *
 *  Created on: Jul 2, 2010
 *      Author: wellska1
 */

#ifndef REQUEST_H_
#define REQUEST_H_

const char* request_get_method();
const char* request_get_url_path();
const char* request_get_path_variable(const char* key);

#endif /* REQUEST_H_ */
