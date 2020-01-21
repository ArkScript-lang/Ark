# Msgpack module docummentation


_For the moment_ we have an implementation with some many limits mainly cause by some
restrictions of API, but we can make basic operations that can be done by the original library, like packing, unpacking and a limited use in background of msgpack objects. So
the list of usable functions is : 

#### **msgPack** : for data packing ;
#### **msgUnpack** : for data unpacking ;
#### **msgObjectStr** : for getting deserialized msgpack object string.

## Example

``` clojure
{
	(import "msgpack.bin")

	# list source
	(let lst (list 1 true "hello" 1.453))
	# list packing
	(let buffer (msgPack lst))
	(print "buffer : " buffer)
	# getting deserialized object string
	(let deserialized (msgObjectStr buffer))
	(print "deserialized : " deserialized)
	# print unpacked 
	(print "unpacked result : " (msgUnpack buffer))
}
```


