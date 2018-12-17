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
<<<<<<< HEAD
	/*
=======
	/* 
>>>>>>> Add initial project files
	 * Implement this. Parts of it are already implemented
	 */
	vector<Node> curMemList;
	bool change = false;

	/*
	 *  Step 1. Get the current membership list from Membership Protocol / MP1
	 */
	curMemList = getMembershipList();

	/*
	 * Step 2: Construct the ring
	 */
	// Sort the list based on the hashCode
	sort(curMemList.begin(), curMemList.end());

	vector<Node> old_ring = ring;

	ring = curMemList;

	/*
	 * Step 3: Run the stabilization protocol IF REQUIRED
	 */
	// Run stabilization protocol if the hash table size is greater than zero and if there has been a changed in the ring
	if(ht->currentSize() > 0 || old_ring != ring){
		stabilizationProtocol();
	}

	// Run stabilization protocol if the hash table size is greater than zero and 
	//if there has been a changed in the ring

	bool is_ring_changes = false;
	if (ring.size == 0) {
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
	Message msg = Message(g_transID, memberNode->addr, READ, key);
	
	vector<Node> replicas = findNodes(key);

	g_transID += 1;
	Message msg = Message(g_transID, memberNode->addr, CREATE, key);
	
	vector<Node> replicas = findNodes(key, ring);

	if (replicas.size() > 0) {
		msg.replica = PRIMARY;
		// use getAddress to return pointer

		emulNet->ENsend(&memberNode->addr, replicas[0].getAddress(), msg.toString());
	}

	if (replicas.size() > 1) {
		msg.replica = SECONDARY;
		emulNet->ENsend(&memberNode->addr, replicas[0].getAddress(), msg.toString());
	}

	if (replicas.size() > 2) {
		msg.replica = TERTIARY;
		emulNet->ENsend(&memberNode->addr, replicas[0].getAddress(), msg.toString());
	}

	replicas.clear();
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


}

/**
 * FUNCTION NAME: createKeyValue
 *
 * DESCRIPTION: Server side CREATE API
 * 			   	The function does the following:
 * 			   	1) Inserts key value into the local hash table
 * 			   	2) Return true or false based on success or failure
 */
bool MP2Node::createKeyValue(string key, string value, ReplicaType replica) {

	/*
	 * Implement this
	 */
	// Insert key, value, replicaType into the hash table
	g_transID += 1;
	vector<Node> replicas = findNodes(key);
	bool is_coordinator = false;


	if (ht->create(key, value)) {
		log->logCreateSuccess(&memberNode->addr, is_coordinator, g_transID, key, value);
		return true;
	} else {
		log->logCreateFail(&memberNode->addr, is_coordinator, g_transID, key, value);
		return false;
	}

}

/**
 * FUNCTION NAME: readKey
 *
 * DESCRIPTION: Server side READ API
 * 			    This function does the following:
 * 			    1) Read key from local hash table
 * 			    2) Return value
 */
string MP2Node::readKey(string key) {
	/*
	 * Implement this
	 */
	// Read key from local hash table and return value
}

/**
 * FUNCTION NAME: updateKeyValue
 *
 * DESCRIPTION: Server side UPDATE API
 * 				This function does the following:
 * 				1) Update the key to the new value in the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::updateKeyValue(string key, string value, ReplicaType replica) {
	/*
	 * Implement this
	 */
	// Update key in local hash table and return true or false
}

/**
 * FUNCTION NAME: deleteKey
 *
 * DESCRIPTION: Server side DELETE API
 * 				This function does the following:
 * 				1) Delete the key from the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::deletekey(string key) {
	/*
	 * Implement this
	 */
	// Delete the key from the local hash table
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

		Message msg = Message(message);

		if (msg.type == CREATE) {

			bool is_success = createKeyValue(msg.key, msg.value, msg.replica, msg.transID);

			nodes_have_key = findNodes(key, ring);

			ReplicaType replica_type = msg.type;

			if (replica_type == PRIMARY){
				
				for (int i=1; i<=2; i++) {
					if (is_present(nodes_have_key.at(i), hasMyReplicas == -1))
						hasMyReplicas.emplace_back(nodes_have_key.at(i));
				}

			} else if (replica_type == SECONDARY) {

				if (is_present(nodes_have_key.at(0), haveReplicasOf == -1)) {
					haveReplicasOf.emplace_back(nodes_have_key.at(0));
				}

			} else{

				for (int i=1; i<=2; i++) {
					if (is_present(nodes_have_key.at(i), haveReplicasOf) == -1)
						haveReplicasOf.emplace_back(nodes_have_key.at(i));
				}
			}

			Message reply_message = Message(msg.transID, memberNode->addr, REPLY, is_success);

			emulNet->ENsend(&memberNode->addr, &msg.fromAddr, rep_message.toString());
		}

	}

	/*
	 * This function should also ensure all READ and UPDATE operation
	 * get QUORUM replies
	 */
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
			for (int i=1; i<ring.size(); i++){
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
<<<<<<< HEAD
 * FUNCTION NAME: stabilizationProtocol
=======
 * FUNCTION NAME: stabilizationProtocol -some nodes fails and some joins 
 					meaning that the membership list changes
 				 as time goes on, so, for example, if a node fails then a replica count for some key-value
 				  decreases and thus you need to add one more replica for that key-value. Some node may join, 
 				so in that case you need kind of rearrange you ring. This process is called "stabilization".
>>>>>>> Add initial project files
 *
 * DESCRIPTION: This runs the stabilization protocol in case of Node joins and leaves
 * 				It ensures that there always 3 copies of all keys in the DHT at all times
 * 				The function does the following:
<<<<<<< HEAD
 *				1) Ensures that there are three "CORRECT" replicas of all the keys in spite of failures and joins
 *				Note:- "CORRECT" replicas implies that every key is replicated in its two neighboring nodes in the ring
 */
// void MP2Node::stabilizationProtocol() {
// 	/*
// 	 * Implement this
// 	 */

 // *				1) Ensures that there are three "CORRECT" replicas of all the keys in spite of failures 
 // *				and joins
 // *				Note:- "CORRECT" replicas implies that every key is replicated in its two neighboring
 // *				 nodes in the ring
//  */
// void MP2Node::stabilizationProtocol(vector<Node> new_ring) {
// 	/*
// 	 * Implement this
// 	 */

// 	Node myself = Node(memberNode->addr);
// 	int my_pos = find_pos(myself, new_ring);
// 	int 1st_replica_pos = (my_pos + 1) % list.size();
// 	int 2nd_repica_pos = (my_pos + 2) % list.size();

// 	if (hasMyReplicas.size() == 0) {
// 		update_key_in_replica(new_ring, new_ring[1st_replica_pos], SECONDARY);
// 		update_key_in_replica(new_ring, new_ring[2nd_replica_pos], TERTIARY);
	
// 	} else {
// 		if (!(ring_to_stabilize(1st_replica_pos).nodeAddress == hasMyReplicas[0].nodeAddress)){
// 			// remove key from old SECONDARY node and update new one
// 			delete_keys_in_replica(new_ring, hasMyReplicas[0]);
// 			update_keys_in_replica(new_ring, new_ring[1st_replica_pos], SECONDARY);
// 		}

// 		if (!(ring_to_stabilize(2nd_replica_pos).nodeAddress == hasMyReplicas[0].nodeAddress)){
// 			// remove key from old SECONDARY node and update new one
// 			delete_keys_in_replica(new_ring, hasMyReplicas[0]);
// 			update_keys_in_replica(new_ring, new_ring[2nd_replica_pos], TERTIARY);
// 		}		
// 	}

// 	int 1st_predecessor_pos = (my_pos + new_ring.size() - 1) % list.size();
// 	int 2nd_predecessor_pos = (my_pos + new_ring.size() - 2) % list.size();

// 	// // if current node is not replica of previous 2 nodes in virtual ring then do nothing, just wait to update
// 	// if (haveReplicasOf.size() != 0) {

// 	// 	if(!(new_ring(1st_predecessor_pos).nodeAddress == haveReplicasOf[0].nodeAddress)) {
// 	// 		update_keys_in_replica(ring_to_stabilize, ring_to_stabilize[1st_replica_pos], TERTIARY);


// 	// 	}

// 	// }





// 	hasMyReplicas.clear();
// 	haveReplicasOf.clear();
// 	hasMyReplicas.push_back(newRing[1st_replica_pos]);
// 	hasMyReplicas.push_back(newRing[2nd_replica_pos]);
// 	haveReplicasOf.push_back(newRing[1st_predecessor_pos]);
// 	haveReplicasOf.push_back(newRing[2nd_predecessor_pos])


// }

// void MP2Node::stabilizationProtocol2(vector<Node> new_ring) {
	
// 	vector<Node> failed_nodes;

// 	for(auto it = ring.begin(); it != ring.end(); ++it) {
		
// 		if(is_present(*it, new_ring) != 1)
// 			failed_nodes.emplace_back(Node((*it).nodeAddress));
// 	}

// 	Node myself = Node(memberNode->addr);
// 	int my_pos = find_pos(myself, new_ring);
// 	int 1st_replica_pos = (my_pos + 1) % list.size();
// 	int 2nd_repica_pos = (my_pos + 2) % list.size();

// 	string key;
// 	string value;
// 	Entry new_entry;

// 	for(auto it = failed_nodes.begin(); it < failed_nodes.end(); ++it) {
// 		if(is_presence(*it, haveReplicasOf)) {

// 			if (haveReplicasOf.at(0).nodeHashCode == *it.getHashCode) {
// 				for (ht_it = ht->hashTable.begin(); ht_it != ht->hashTable.end(); ++ht_it) {
// 					key = ht_it->first;
// 					value = ht_it->second;
// 					new_entry = Entry(value);
// 					if (new_entry->replica == SECONDARY) {
// 						g_transID += 1;
// 						Message reply_msg = new Message(g_transID, memberNode->addr, CREATE, key, new_entry->value, TERTIARY);
// 						emulNet->ENsend(&memberNode->addr, &new_ring.at(2nd_replica_pos).nodeAddress, rep_message);
// 					}
// 				}	
// 			}
// 		}
// 	}


// }

void MP2Node::stabilizationProtocol(vector<Node> new_ring) {

	for (auto it=ht->hashTable.begin(); it != ht->HashTable.end(); ++it) {

		string key = it->first;
		string value = it->second;

		vector<Node> new_replicas_of_key = findNodes(key, new_ring);
		vector<Node> old_replicas_of_key = findNodes(key, old_ring);

	    int pos = -1;
	    for(int i = 0 ; i< 3;i++) {
	    	if (new_replicas_of_key[i].nodeAddress == memberNode->addr)
	        	pos = i;
	    }
	    
	    g_transID += 1;

		if (pos == -1) {

			// the key does not belong to this node anymore
			update_replica(new_replicas_of_key.at(0).nodeAddress, key, value, CREATE, PRIMARY);
        	update_replica(new_replicas_of_key.at(1).nodeAddress, key, value, CREATE, SECONDARY);
        	update_replica(new_replicas_of_key.at(2).nodeAddress, key, value, CREATE ,TERTIARY);
        	
        	// delete the key from my self but do not delete others as they will self-deleted
        	ht->deleteKey(key);

		} else if (pos == 0) {
			// the key is primary in this node

			// step 1: update this key as primary to myself
			update_replicate(memberNode->addr, key, value, PRIMARY);

			// Step 2: create this key as SECONDARY to next replica
			if (!(old_replicas_of_key.at(1).nodeAddress == new_replicas_of_key.at(1).nodeAddress))
				update_replica(new_replicas_of_key.at(1).nodeAddress, key, value, CREATE,SECONDARY);
			
			// Step 3: create this key as TERTIARY to next replica
			if (!(old_replicas_of_key.at(2).nodeAddress == new_replicas_of_key.at(2).nodeAddress))
				update_replica(new_replicas_of_key.at(2).nodeAddress, key, value, CREATE, TERTIARY);
		
		} else {

			int new_pos = my_position(memberNode->addr, new_ring);	
			int new_1st_predecessor_pos = (new_pos + new_ring.size() - 1) % new_ring.size();
			int new_2nd_predecessor_pos = (new_pos + new_ring.size() - 2) % new_ring.size();

			int old_pos = my_position(old_ring);		
			int old_1st_predecessor_pos = (pos + old_ring.size() - 1) % old_ring.size();
			int old_2nd_predecessor_pos = (pos + old_ring.size() - 2) % old_ring.size();

			if (pos == 1) {
				// the key is secondary in this node

				// step 1: update this key as SECONDARY to myself
				update_replicate(memberNode->addr, key, value, SECONDARY);

				// Step 2: create this key as TERTIARY to next replica
				if (!(old_replicas_of_key.at(1).nodeAddress == new_replicas_of_key.at(1).nodeAddress)) {
					update_replica(new_replicas_of_key.at(1).nodeAddress, key, value, CREATE, TERTIARY);
				}

				// Step 3: 	create this key as PRIMARY to previous predecessor
				Address old_1st_predecessor_addr = old_ring[old_1st_predecessor_addr].nodeAddress;
				Address new_1st_predecessor_addr = new_ring[new_1st_predecessor_addr].nodeAddress;

				if (!(old_1st_predecessor_addr == new_1st_predecessor_addr)) {
					update_replica(new_1st_predecessor_addr, key, value, CREATE, PRIMARY);
				}	
				
			} else if (pos == 2) {
				// the key is TERTIARY in this node

				// step 1: update this key as TERTIARY to myself
				update_replica(memberNode->addr, key, value, UPDATE, SECONDARY);

				// step 2: create this key as PRIMARY to 2nd predecessor
				Address old_2nd_predecessor_addr = old_ring[old_2nd_predecessor_addr].nodeAddress;
				Address new_2nd_predecessor_addr = new_ring[new_2nd_predecessor_addr].nodeAddress;
				
				if (!(old_2nd_predecessor_addr == new_end_predecessor_addr)) {
					update_replica(new_2nd_predecessor_addr, key, value, CREATE, PRIMARY);
				}

				// step 2: create this key as PRIMARY to 2nd predecessor
				Address old_1st_predecessor_addr = old_ring[old_1st_predecessor_addr].nodeAddress;
				Address new_1st_predecessor_addr = new_ring[new_1st_predecessor_addr].nodeAddress;
				
				if (!(old_1st_predecessor_addr == new_1st_predecessor_addr)) {
					update_replica(g_transID, new_1st_predecessor_addr, key, value, CREATE, PRIMARY);
				}

			}
		}
	}

}


int MP2Node::find_position(Node node, std::vector<Node> list) {

	for (int i = 0; i<list.size(); i++) {
		if (node.nodeHashCode == list[i].nodeHashCode)
			return i;
	}
	return -1;
}

void MP2Node::update_replica(int transID, Address addr, string key, string value,  MessageType message_type, ReplicaType replica_type) {
	
	Message message = Message(transID, memberNode->addr, message_type, key, value, replica_type);
	emulNet->ENsend(&memberNode->addr, &addr, message.toString());
}

