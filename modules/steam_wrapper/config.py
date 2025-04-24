def can_build(env, platform):
	return True # we don't need to validate os here, module itself do that at construction time
				# that also allow that we can avoid Engine.has_singleton() validations in gdscript.

def configure(env):
	pass

def get_doc_classes():
	return [
		"SteamWrapper",
	]

def get_doc_path():
	return "doc_classes"
