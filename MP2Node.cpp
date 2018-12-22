/**********************************
 * FILE NAME: MP2Node.cpp
 *
 * DESCRIPTION: MP2Node class definition
 **********************************/
#include "MP2Node.h"

/**
 * constructor
 */
MP2Node::MP2Node(Member *memberNode, Params *par, EmulNet * emulNet, Log * log, Address * address) {
	this->memberNode = memberNode;
	this->par = par;
	this->emulNet = emulNet;
	this->log = log;
	ht = new HashTable();
	this->memberNode->addr = *address;
}

/**
 * Destructor
 */
MP2Node::~MP2Node() {
	delete ht;
	delete memberNode;
}

/**
 * FUNCTION NAME: updateRing
 *
 * DESCRIPTION: This function does the following:
 * 				1) Gets the current membership list from the Membership Protocol (MP1Node)
 * 				   The membership list is returned as a vector of Nodes. See Node class in Node.h
 * 				2) Constructs the ring based on the membership list
 * 				3) Calls the Stabilization Protocol
 */
void MP2Node::updateRing() {
	/*
	 * Implement this. Parts of it are already implemented
	 */
	vector<Node> curMemList;

	/*
	 *  Step 1. Get the current membership list from Membership Protocol / MP1
	 */
	curMemList = getMembershipList();

	/*
	 * Step 2: Construct the ring
	 */
	// Sort the list based on the hashCode
	sort(curMemList.begin(), curMemList.end());


	/*
	 * Step 3: Run the stabilization protocol IF REQUIRED
	 */
	// Run stabilization protocol if the hash table size is greater than zero and 
	//if there has been a changed in the ring

	bool is_ring_changes = false;
	if (ring.size() == 0) {
		ring = curMemList;
	} else {
		is_ring_changes = ring_changed(ring, curMemList);
	}

	if (ht->currentSize() > 0 && is_ring_changes){
		stabilizationProtocol(curMemList); // run stability protocol for curMemList
		ring = curMemList;
	}

	sort(ring.begin(), ring.end());

}

/**
 * FUNCTION NAME: getMemberhipList
 *
 * DESCRIPTION: This function goes through the membership list from the Membership protocol/MP1 and
 * 				i) generates the hash code for each member
 * 				ii) populates the ring member in MP2Node class
 * 				It returns a vector of Nodes. Each element in the vector contain the following fields:
 * 				a) Address of the node
 * 				b) Hash code obtained by consistent hashing of the Address
 */
vector<Node> MP2Node::getMembershipList() {
	unsigned int i;
	vector<Node> curMemList;
	for ( i = 0 ; i < this->memberNode->memberList.size(); i++ ) {
		Address addressOfThisMember;
		int id = this->memberNode->memberList.at(i).getid();
		short port = this->memberNode->memberList.at(i).getport();
		memcpy(&addressOfThisMember.addr[0], &id, sizeof(int));
		memcpy(&addressOfThisMember.addr[4], &port, sizeof(short));
		curMemList.emplace_back(Node(addressOfThisMember));
	}
	return curMemList;
}

/**
 * FUNCTION NAME: hashFunction
 *
 * DESCRIPTION: This functions hashes the key and returns the position on the ring
 * 				HASH FUNCTION USED FOR CONSISTENT HASHING
 *
 * RETURNS:
 * size_t position on the ring
 */
size_t MP2Node::hashFunction(string key) {
	std::hash<string> hashFunc;
	size_t ret = hashFunc(key);
	return ret%RING_SIZE;
}

/**
 * FUNCTION NAME: clientCreate
 *
 * DESCRIPTION: client side CREATE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientCreate(string key, string value) {
	/*
	 * Implement this
	 */
	// increase transaction ID before perform operation

	g_transID += 1;
	Message* send_msg = new Message(g_transID, memberNode->addr, CREATE, key, value);
	string message = send_msg->toString();

	operation_table[g_transID] = message;

	vector<Node> replicas = findNodes(key);

	for (int i = 0; i < 3; i++) {
		send_msg->replica = ReplicaType(i);
		emulNet->ENsend(&(send_msg->fromAddr), &(replicas[i].nodeAddress), message);
	}

	free(send_msg);
}

/**
 * FUNCTION NAME: clientRead
 *
 * DESCRIPTION: client side READ API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientRead(string key){
	/*
	 * Implement this
	 */
	g_transID += 1;

	Message *send_msg = new Message(g_transID, memberNode->addr, READ, key);
	vector<Node> replicas = findNodes(key);
	string message = send_msg->toString();

	operation_table[g_transID] = message;

	emulNet->ENsend(&(send_msg->fromAddr), &(replicas[0].nodeAddress), message);
	free(send_msg);
}

/**
 * FUNCTION NAME: clientUpdate
 *
 * DESCRIPTION: client side UPDATE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientUpdate(string key, string value){
	/*
	 * Implement this
	 */
	g_transID += 1;

	Message* send_msg = new Message(g_transID, memberNode->addr, UPDATE, key, value);
	string message = send_msg->toString();

	operation_table[g_transID] = message;

	std::vector<Node> replicas = findNodes(key);

	for (int i = 0; i < 3; i++) {
		send_msg->replica = ReplicaType(i);
		emulNet->ENsend(&(send_msg->fromAddr), &(replicas[i].nodeAddress), message);
	}

	free(send_msg);
}

/**
 * FUNCTION NAME: clientDelete
 *
 * DESCRIPTION: client side DELETE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientDelete(string key){
	/*
	 * Implement this
	 */

	g_transID += 1;

	Message* send_msg = new Message(g_transID, memberNode->addr, DELETE, key);
	string message = send_msg->toString();

	operation_table[g_transID] = message;

	std::vector<Node> replicas = findNodes(key);

	for (int i = 0; i < 3; i++) {
		send_msg->replica = ReplicaType(i);
		emulNet->ENsend(&(send_msg->fromAddr), &(replicas[i].nodeAddress), message);
	}
	free(send_msg);
}

/**
 * FUNCTION NAME: createKeyValue
 *
 * DESCRIPTION: Server side CREATE API
 * 			   	The function does the following:
 * 			   	1) Inserts key value into the local hash table
 * 			   	2) Return true or false based on success or failure
 */
bool MP2Node::createKeyValue(string key, string value, ReplicaType replica, int transID) {
	/*
	 * Implement this
	 */
	// Insert key, value, replicaType into the hash table

	Entry* entry = new Entry(value, par->getcurrtime(), replica);

	bool is_success = ht->create(key, entry->convertToString());

	if (is_success) {
		log->logCreateSuccess(&memberNode->addr, false, transID, key, value);
	} else {
		log->logCreateFail(&memberNode->addr, false, transID, key, value);
	}

	return is_success;
}

/**
 * FUNCTION NAME: readKey
 *
 * DESCRIPTION: Server side READ API
 * 			    This function does the following:
 * 			    1) Read key from local hash table
 * 			    2) Return value
 */
string MP2Node::readKey(string key, int transID) {
	/*
	 * Implement this
	 */
	// Read key from local hash table and return value
	
	string value = ht->read(key);
	
	if (value != "") {
		log->logReadSuccess(&memberNode->addr, false, transID, key, value);
	} else {
		log->logReadFail(&memberNode->addr, false, transID, key);
	}

	return value;
}

/**
 * FUNCTION NAME: updateKeyValue
 *
 * DESCRIPTION: Server side UPDATE API
 * 				This function does the following:
 * 				1) Update the key to the new value in the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::updateKeyValue(string key, string value, ReplicaType replica, int transID) {
	/*
	 * Implement this
	 */
	// Update key in local hash table and return true or false

	Entry* entry = new Entry(value, par->getcurrtime(), replica);

	bool is_sucess = ht->update(key, entry->convertToString());
	
	if (is_sucess) {
		log->logUpdateSuccess(&memberNode->addr, false, transID, key, value);
	} else {
		log->logUpdateFail(&memberNode->addr, false, transID, key, value);
	}

	return is_sucess;
}

/**
 * FUNCTION NAME: deleteKey
 *
 * DESCRIPTION: Server side DELETE API
 * 				This function does the following:
 * 				1) Delete the key from the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::deletekey(string key, int transID) {
	/*
	 * Implement this
	 */
	// Delete the key from the local hash table

	bool is_sucess = ht->deleteKey(key);
	
	if (is_sucess) {
		log->logDeleteSuccess(&memberNode->addr, false, transID, key);
	} else {
		log->logDeleteFail(&memberNode->addr, false, transID, key);
	}

	return is_sucess;

}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: This function is the message handler of this node.
 * 				This function does the following:
 * 				1) Pops messages from the queue
 * 				2) Handles the messages according to message types
 */
void MP2Node::checkMessages() {
	/*
	 * Implement this. Parts of it are already implemented
	 */
	char * data;
	int size;

	/*
	 * Declare your local variables here
	 */
	Message* msg;
	Message* reply_message;
	MessageType msg_type;
	bool is_success; 
	int reply_transID;

	// dequeue all messages and handle them
	while ( !memberNode->mp2q.empty() ) {
		/*
		 * Pop a message from the queue
		 */
		data = (char *)memberNode->mp2q.front().elt;
		size = memberNode->mp2q.front().size;
		memberNode->mp2q.pop();

		string message(data, data + size);

		/*
		 * Handle the message types here
		 */
		msg = new Message(message);
		msg_type = msg->type;
		reply_transID = msg->transID;

		if (msg_type == CREATE) {

			is_success = createKeyValue(msg->key, msg->value, msg->replica, reply_transID);
			vector<Node> nodes_have_key = findNodes(msg->key);

			ReplicaType replica_type = msg->replica;

			if (replica_type == PRIMARY){
				
				// add nodes into hasmyreplica
				for (int i=1; i <= 2; i++) {
					if (find_position(nodes_have_key.at(i), hasMyReplicas) == -1)
						hasMyReplicas.emplace_back(nodes_have_key.at(i));
				}

			} else if (replica_type == SECONDARY) {

				if (find_position(nodes_have_key.at(0), haveReplicasOf) == -1) {
					haveReplicasOf.emplace_back(nodes_have_key.at(0));
				}

				if (find_position(nodes_have_key.at(2), hasMyReplicas) == -1) {
					hasMyReplicas.emplace_back(nodes_have_key.at(0));
				}			

			} else {

				for (int i=1; i<=2; i++) {
					if (find_position(nodes_have_key.at(i), haveReplicasOf) == -1)
						haveReplicasOf.emplace_back(nodes_have_key.at(i));
				}
			}

			reply_message = new Message(reply_transID , memberNode->addr, REPLY, is_success);
			emulNet->ENsend(&memberNode->addr, &msg->fromAddr, reply_message->toString());
		
		} else if (msg_type == DELETE) {

			is_success = deletekey(msg->key, msg->transID);
			reply_message = new Message(reply_transID, memberNode->addr, REPLY, is_success);
			emulNet->ENsend(&memberNode->addr, &msg->fromAddr, reply_message->toString());
		
		} else if (msg_type == READ) {

			string value = readKey(msg->key, reply_transID);

			// reply message of read is different from others
			reply_message = new Message(reply_transID, memberNode->addr, value);
			emulNet->ENsend(&memberNode->addr, &msg->fromAddr, reply_message->toString());
			
		} else if (msg_type == UPDATE) {

			// update message has replica type
			is_success = updateKeyValue(msg->key, msg->value, msg->replica, reply_transID);
			reply_message = new Message(reply_transID, memberNode->addr, REPLY, is_success);
			emulNet->ENsend(&memberNode->addr, &msg->fromAddr, reply_message->toString());			
		
		} else if (msg_type == REPLY) {

			// get the success from reply message
			is_success = msg->success;

			// key already present
			if (result_table.find(reply_transID) != result_table.end()) {
				result_table[reply_transID].emplace_back(is_success);
			} else { // key not present yet
				vector<bool> new_result;
				new_result.emplace_back(is_success);
				result_table[reply_transID] = new_result;
			}

		} else if (msg_type == READREPLY) {
			string reply_value = msg->value;

			// key already present
			if (read_value_table.find(reply_transID) != read_value_table.end()) {
				read_value_table[reply_transID].emplace_back(reply_value);
			} else { // key not present yet
				vector<string> new_value;
				new_value.emplace_back(reply_value);
				read_value_table[reply_transID] = new_value;
			}
		}
		
	} // end of while loop

	free(msg);
	free(reply_message);

	/*
	 * This function should also ensure all READ and UPDATE operation
	 * get QUORUM replies
	 */

	/*checking quorum for UPDATE, DELETE and CREATE operation*/
	check_update_operations();

	/*checking quorum for READ operation*/
	check_read_operations();
}

/**
 * FUNCTION NAME: findNodes
 *
 * DESCRIPTION: Find the replicas of the given keyfunction
 * 				This function is responsible for finding the replicas of a key
 */
vector<Node> MP2Node::findNodes(string key) {
	size_t pos = hashFunction(key);
	vector<Node> addr_vec;
	if (ring.size() >= 3) {
		// if pos <= min || pos > max, the leader is the min
		if (pos <= ring.at(0).getHashCode() || pos > ring.at(ring.size()-1).getHashCode()) {
			addr_vec.emplace_back(ring.at(0));
			addr_vec.emplace_back(ring.at(1));
			addr_vec.emplace_back(ring.at(2));
		}
		else {
			// go through the ring until pos <= node
			for (unsigned i=1; i<ring.size(); i++){
				Node addr = ring.at(i);
				if (pos <= addr.getHashCode()) {
					addr_vec.emplace_back(addr);
					addr_vec.emplace_back(ring.at((i+1)%ring.size()));
					addr_vec.emplace_back(ring.at((i+2)%ring.size()));
					break;
				}
			}
		}
	}
	return addr_vec;
}

/**
 * FUNCTION NAME: findNodes
 *
 * DESCRIPTION: Find the replicas of the given keyfunction
 * 				This function is responsible for finding the replicas of a key
 */
vector<Node> MP2Node::findNodes(string key, std::vector<Node> ring) {
	size_t pos = hashFunction(key);
	vector<Node> addr_vec;
	if (ring.size() >= 3) {
		// if pos <= min || pos > max, the leader is the min
		if (pos <= ring.at(0).getHashCode() || pos > ring.at(ring.size()-1).getHashCode()) {
			addr_vec.emplace_back(ring.at(0));
			addr_vec.emplace_back(ring.at(1));
			addr_vec.emplace_back(ring.at(2));
		}
		else {
			// go through the ring until pos <= node
			for (unsigned i=1; i<ring.size(); i++){
				Node addr = ring.at(i);
				if (pos <= addr.getHashCode()) {
					addr_vec.emplace_back(addr);
					addr_vec.emplace_back(ring.at((i+1)%ring.size()));
					addr_vec.emplace_back(ring.at((i+2)%ring.size()));
					break;
				}
			}
		}
	}
	return addr_vec;
}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: Receive messages from EmulNet and push into the queue (mp2q)
 */
bool MP2Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), this->enqueueWrapper, NULL, 1, &(memberNode->mp2q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue of MP2Node
 */
int MP2Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}
/**
 * FUNCTION NAME: stabilizationProtocol -some nodes fails and some joins 
 					meaning that the membership list changes
 				 as time goes on, so, for example, if a node fails then a replica count for some key-value
 				  decreases and thus you need to add one more replica for that key-value. Some node may join, 
 				so in that case you need kind of rearrange you ring. This process is called "stabilization".
 *
 * DESCRIPTION: This runs the stabilization protocol in case of Node joins and leaves
 * 				It ensures that there always 3 copies of all keys in the DHT at all times
 * 				The function does the following:
 *				1) Ensures that there are three "CORRECT" replicas of all the keys in spite of failures 
 *				and joins
 *				Note:- "CORRECT" replicas implies that every key is replicated in its two neighboring
 *				 nodes in the ring
//  */

void MP2Node::stabilizationProtocol(vector<Node> new_ring) {

	for (auto it=ht->hashTable.begin(); it != ht->hashTable.end(); ++it) {

		string key = it->first;
		string value = it->second;

		vector<Node> new_replicas_of_key = findNodes(key, new_ring);
		vector<Node> old_replicas_of_key = findNodes(key, ring);

	    int pos = -1;
	    for(int i = 0 ; i< 3;i++) {
	    	if (new_replicas_of_key[i].nodeAddress == memberNode->addr)
	        	pos = i;
	    }
	    
	    g_transID += 1;

		if (pos == -1) {

			// the key does not belong to this node anymore
			update_replica(g_transID, new_replicas_of_key.at(0).nodeAddress, key, value, CREATE, PRIMARY);
        	update_replica(g_transID, new_replicas_of_key.at(1).nodeAddress, key, value, CREATE, SECONDARY);
        	update_replica(g_transID, new_replicas_of_key.at(2).nodeAddress, key, value, CREATE ,TERTIARY);
        	
        	// delete the key from my self but do not delete others as they will self-deleted
        	ht->deleteKey(key);

		} else if (pos == 0) {
			// the key is primary in this node

			// step 1: update this key as primary to myself
			update_replica(g_transID, memberNode->addr, key, value, UPDATE, PRIMARY);

			// Step 2: create this key as SECONDARY to next replica
			if (!(old_replicas_of_key.at(1).nodeAddress == new_replicas_of_key.at(1).nodeAddress))
				update_replica(g_transID, new_replicas_of_key.at(1).nodeAddress, key, value, CREATE,SECONDARY);
			
			// Step 3: create this key as TERTIARY to next replica
			if (!(old_replicas_of_key.at(2).nodeAddress == new_replicas_of_key.at(2).nodeAddress))
				update_replica(g_transID, new_replicas_of_key.at(2).nodeAddress, key, value, CREATE, TERTIARY);
		
		} else {

			int new_pos = find_position(memberNode->addr, new_ring);	
			int new_1st_predecessor_pos = (new_pos + new_ring.size() - 1) % new_ring.size();
			int new_2nd_predecessor_pos = (new_pos + new_ring.size() - 2) % new_ring.size();

			int old_pos = find_position(memberNode->addr, ring);		
			int old_1st_predecessor_pos = (old_pos + ring.size() - 1) % ring.size();
			int old_2nd_predecessor_pos = (old_pos + ring.size() - 2) % ring.size();

			if (pos == 1) {
				// the key is secondary in this node

				// step 1: update this key as SECONDARY to myself
				update_replica(g_transID, memberNode->addr, key, value, UPDATE, SECONDARY);

				// Step 2: create this key as TERTIARY to next replica
				if (!(old_replicas_of_key.at(1).nodeAddress == new_replicas_of_key.at(1).nodeAddress)) {
					update_replica(g_transID, new_replicas_of_key.at(1).nodeAddress, key, value, CREATE, TERTIARY);
				}

				// Step 3: 	create this key as PRIMARY to previous predecessor
				Address old_1st_predecessor_addr = ring[old_1st_predecessor_pos].nodeAddress;
				Address new_1st_predecessor_addr = new_ring[new_1st_predecessor_pos].nodeAddress;

				if (!(old_1st_predecessor_addr == new_1st_predecessor_addr)) {
					update_replica(g_transID, new_1st_predecessor_addr, key, value, CREATE, PRIMARY);
				}	
				
			} else if (pos == 2) {
				// the key is TERTIARY in this node

				// step 1: update this key as TERTIARY to myself
				update_replica(g_transID, memberNode->addr, key, value, UPDATE, SECONDARY);

				// step 2: create this key as PRIMARY to 2nd predecessor
				Address old_2nd_predecessor_addr = ring[old_2nd_predecessor_pos].nodeAddress;
				Address new_2nd_predecessor_addr = new_ring[new_2nd_predecessor_pos].nodeAddress;
				
				if (!(old_2nd_predecessor_addr == new_2nd_predecessor_addr)) {
					update_replica(g_transID, new_2nd_predecessor_addr, key, value, CREATE, PRIMARY);
				}

				// step 2: create this key as PRIMARY to 2nd predecessor
				Address old_1st_predecessor_addr = 	ring.at(old_1st_predecessor_pos).nodeAddress;
				Address new_1st_predecessor_addr = new_ring.at(new_1st_predecessor_pos).nodeAddress;
				
				if (!(old_1st_predecessor_addr == new_1st_predecessor_addr)) {
					update_replica(g_transID, new_1st_predecessor_addr, key, value, CREATE, PRIMARY);
				}

			}
		}
	}

}


int MP2Node::find_position(Node node, vector<Node> list) {

	for (unsigned i = 0; i<list.size(); i++) {
		if (node.nodeHashCode == list[i].nodeHashCode)
			return i;
	}
	return -1;
}

int MP2Node::find_position(Address addr, vector<Node> list) {

	for (unsigned i = 0; i<list.size(); i++) {
		if (addr == list[i].nodeAddress)
			return i;
	}
	return -1;
}



void MP2Node::update_replica(int transID, Address addr, string key, string value,  MessageType message_type, ReplicaType replica_type) {
	
	Message message = Message(transID, memberNode->addr, message_type, key, value, replica_type);
	emulNet->ENsend(&memberNode->addr, &addr, message.toString());
}

bool MP2Node::ring_changed(vector<Node> ring1, vector<Node> ring2) {

	if (ring1.size() != ring2.size()) 
		return true;

	for (auto it1 = ring1.begin() ; it1 < ring1.end(); ++it1) {
		for (auto it2 = ring2.begin(); it2 < ring2.end(); ++it2) {
			if ((*it1).nodeHashCode == (*it2).nodeHashCode) {
				return true;
			}
		}
	}

	return false;
}

void MP2Node::check_read_operations() {
	Message* received_msg;
	Entry *entryObj;
	string reply_value;
	int reply_transID;
	Address reply_from;

	if (read_value_table.size() > 0) {

		for (auto it = read_value_table.begin(); it != read_value_table.end(); ++it) {

			reply_transID = it->first;

			if (operation_table.find(reply_transID) != operation_table.end()) {		

				received_msg = new Message(operation_table[reply_transID]);
				reply_from = received_msg->fromAddr;
				vector<string> value_received_from_read = read_value_table[reply_transID];

				vector<string> values;
				for (auto it=value_received_from_read.begin(); it!=value_received_from_read .end(); ++it) {
					if (*it != "") {
						entryObj = new Entry(*it);
						values.push_back(entryObj->value);
					} else
						values.push_back("");			
				}
				
				reply_value = "";
				vector<string> value2;
				for (auto it=value_received_from_read.begin(); it != value_received_from_read.end(); ++it) {
					if (*it != "") {
						entryObj = new Entry(*it);
						value2.push_back(entryObj->value);
					}			
				}				
				if (value2.size() != 0)
					reply_value = value2.at(0);
				

				int counter = 0;
				for (unsigned i = 0; i < values.size(); i++) {
					if (values[i] == reply_value)
						counter += 1;	
				}

				if (received_msg->type == READ) {
					if (counter > 1 && reply_value != "") {
						log->logReadSuccess(&reply_from, true, reply_transID, received_msg->key, reply_value);
					} else {
						log->logReadFail(&reply_from, true, reply_transID, received_msg->key);
					}
				}

			}
		}	
	}
	//free(entryObj);
	//free(received_msg);
	read_value_table.clear();
}

void MP2Node::check_update_operations() {
	Message* received_msg;
	int reply_transID;
	MessageType msg_type;
	Address reply_from;

	if (result_table.size() > 0) {

		for (auto it = result_table.begin(); it != result_table.end(); ++it) {
			reply_transID = it->first;

			if (operation_table.find(reply_transID) != operation_table.end()) {
				
				received_msg = new Message(operation_table[reply_transID]);
				msg_type =  received_msg->type;
				reply_from = received_msg->fromAddr;

				vector<bool> received_success_message = result_table[reply_transID];
				int counter = 0;
				for (unsigned i=0; i < received_success_message.size(); i++) {
					if (received_success_message[i] == true)
						counter += 1;
				}

				if (msg_type == CREATE) {
					if (counter > 1)
						log->logCreateSuccess(&reply_from, true, reply_transID, received_msg->key, received_msg->value);
					else
						log->logCreateFail(&reply_from, true, reply_transID, received_msg->key, received_msg->value);
				
				} else if (msg_type == UPDATE) {
					if (counter > 1)
						log->logUpdateSuccess(&reply_from, true, reply_transID, received_msg->key, received_msg->value);
					else
						log->logUpdateFail(&reply_from, true, reply_transID, received_msg->key, received_msg->value);
				
				} else if (msg_type == DELETE) {
					if (counter > 1)
						log->logDeleteSuccess(&reply_from, true, reply_transID, received_msg->key);
					else
						log->logDeleteFail(&reply_from, true, reply_transID, received_msg->key);
				}

			}
		}
	}
	//free(received_msg);
	result_table.clear();
}


