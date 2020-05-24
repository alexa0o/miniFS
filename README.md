## MiniFS
![](https://travis-ci.org/alexa0o/miniFS.svg?branch=master)  
Just a simple implementation of i-node file system. Also there are client-server
app and server is a daemon.   
List of commands:
1. ``create root/path_to_new_file new_file_name CONTENT``  
2. ``cat root/path_to_file``  
3. ``mkdir root/path_to_new_dir new_dir_name``
4. ``ls root/path``  
5. ``rm root/path_to_removable_file``  
6. ``rmdir root/path_to_removable_dir``  
7. ``put path_on_local root/path_to_new_file``  
8. ``get root/path_to_file path_on_local``

#### Example:
``:> ls root``  
``.``  
``..``

``:> cat root/directory1/directory2/filename``  
``file content!``

``:> rm root/filename``

``:> put Makefile root/dir/Makefile111``

``:> get root/dir/Makefile111 Makefile``

``:> mkdir root/dir1 dir2``

``:> ls root/dir1``  
``.``  
``..``  
``dir2``