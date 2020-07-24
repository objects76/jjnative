
import process from "process";
import { overrideConsole } from "./Logger";
import addon, { bigintFromHandle, NativeAddon } from "../index";

overrideConsole(true);

function test_main() {
  console.log("node version = ", process.versions.node);
  console.log("node_env=", process.env.NODE_ENV);

  let buf8 = Buffer.from([0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xab]);
  let buf4 = Buffer.from([0x00, 0x00, 0x00, 0xff]);
  console.log(buf8, "0x" + bigintFromHandle(buf8).toString(16));
  console.log(buf4, "0x" + bigintFromHandle(buf4).toString(16));

  try {
    // startKeyMonitor with invalid window handle.
    addon.startKeyMonitor(BigInt(1));
  } catch (error) {
    console.error("startKeyMonitor exception: ", error);
  }
}
//test_main();


async function test_native() {

  addon.init((console as any).log_native, null);
  //addon.init(null, null);

  // passing array test
  {
    const array = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);
    try {
      console.log("ArrayBufferArgument=", addon.ArrayBufferArgument(array.buffer));
      console.log("ArrayBufferArgument=", addon.ArrayBufferArgument(array.buffer));

    } catch (error) {
      console.error(error);
    }
    return;
  }

  // promise test.
  console.log("result=", await addon.getPrimeAsync());
  console.log(addon.getPrimeSync());

  // class from addon.
  const obj = new addon.NativeAddon(
    () => { console.log('Hi! I\'m a JSRef function!') },
    () => { console.log('Hi! I\'m a JS function!') }
  );

  // static member funcion test.
  (addon.NativeAddon as any).staticMethod1(obj, "using staticMethod1");
  try {
    (addon.NativeAddon as any).staticMethod_Invalid(obj, "using staticMethod_Invalid");
  } catch (error) {
    console.error(error);
  }

  // member function test.
  console.log(obj.tryCallByStoredReference());
  try {
    //console.log(obj.tryCallByStoredFunction());
  } catch (err) {
    console.error(err);
    console.error('This code crashes because JSFn is valid only inside the constructor function.')
    console.error('The lifespan of the JSFn function is limited to the execution of the constructor function.')
    console.error('After that, the function stored in JSFn is ready to be garbage collected and it cannot be used anymore.')
  }
}

test_native();
