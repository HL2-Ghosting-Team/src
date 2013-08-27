
from twisted.internet.protocol import DatagramProtocol
from twisted.internet import reactor

import struct

# Constants
PASSWORD = "godiebarney"

class HL2RaceGhostHandler(DatagramProtocol):

	# Called on server startup (specifically when we've started to listen)
	def startProtocol(self):
		self.players = {}
		self.inRace = False
		
	# Handle receieved packets
	def datagramReceived(self, data, (host, port)):
		print data
		# Get packet ID
		pID = self.getPacketID(data)
		# First check if its a communication or data packet
		if pID > 127:
			# Position code will go here
			if self.inRace and (host, port) in self.players and pID == 128:
				position_data = struct.unpack("Bfff", data)
				
				print "X: %f Y: %f Z: %f" % position_data[0], position_data[1], position_data[2]
				
				# Relay position data to everyone else in your map
				for player in self.players.values():
					if player["map"] == self.players[(host, port)]["map"]:
						tosend = struct.pack("B" + str(len(self.player[(host, port)]["name"])) + "sBfff", 128, self.player[(host, port)]["name"], 0, position_data[1], position_data[2], position_data[3])
						self.transport.write(tosend, (host, port))
			
		else:
			# Communcation code goes here
			if pID == 0:
				# Do not let any connections in if the race has started.
				# Connect / Disconnect handling
				pass_attempt = struct.unpack(str(len(data) - 1) + "s", data[1:])[0]
				if pass_attempt == PASSWORD:
					if not self.inRace:
						self.players[(host, port)] = {}
						self.transport.write(struct.pack("BB", 0, 1), (host, port))
						print host + " has connected."
						
					else:
						if (host, port) in self.players:
							self.transport.write(struct("BB", 0, 1), (host, port))
						else:
							self.transport.write(struct.pack("BB", 0, 0), (host, port))

				else:
					try:
						del self.players[(host, port)]
					except:
						pass

					finally:
						self.transport.write(struct.pack("BB", 0, 0), (host, port))
						print host + " has disconnected."

		   # The following packet checks should only work if the client has "connected"
			if (host, port) in self.players:
				if pID == 1:
					# Map change
					map_change = struct.unpack(str(len(data)-1) + "s", data[1:])[0]
					self.players[(host, port)]["map"] = map_change
					print host + " has changed to map " + map_change + "."

				if pID == 2:
					# Model / Name change
					if not self.inRace:
						# We're going to split the data by a null terminator to get
						# both strings separately
						split_data = data[1:].split("\x0A")

						model = struct.unpack(str(len(split_data[0])) + "s", split_data[0])[0]
						name = struct.unpack(str(len(split_data[1])) + "s", split_data[1])[0]

						print host + " has changed name to " + name + " and model to " + model + "."
						self.players[(host, port)]["model"] = model
						self.players[(host, port)]["name"] = name
						self.startRace()

	# Convenience functions go here
	def getPacketID(self, packet):
		return struct.unpack("B", packet[0])[0]
		
	def startRace(self):
		# We should check that all the players are ready, if not dc them
		# We should set the inRace variable
		# We should sent the race start packet
		for player in self.players:
			if "map" not in self.players[player].keys() or "name" not in self.players[player].keys() or "model" not in self.players[player].keys():
				self.transport.write(struct.pack("BB", 0, 0), (host, port))
				del self.players[player]
		
		self.inRace = True
		format_string = "B"
		to_pack = [1]
		for player in self.players:
			format_string += str(len(self.players[player]["name"])) + "sB"
			to_pack.append(self.players[player]["name"])
			to_pack.append(0)
			format_string += str(len(self.players[player]["model"])) + "sB"
			to_pack.append(self.players[player]["model"])
			to_pack.append(0)
	
		tosend = struct.pack(format_string, *to_pack)
		
		for player in self.players:
			self.transport.write(tosend, player)
		

reactor.listenUDP(6969, HL2RaceGhostHandler())
reactor.run()
