# Msgpack module docummentation


_For the moment_ we have an implementation with too many limits mainly cause by some
restriction of API, but we can make basic and simple action which can do by the original
library, like packing, unpacking and a limited use in background of msgpack objects. So
the list of usable functions are : 

## **msgPack** : for data packing ;
## **msgUnpack** : for data unpacking ;
## **msgObjectStr** : for getting deserialized msgpack object string.

## Example

``` clojure
{
	(import "libmsgpack.so")

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


